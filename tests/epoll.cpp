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
    template<bool setError, bool hasThen, class... Ts>
    struct [[nodiscard]] GroupExecutorBuilderImpl
    {
        explicit GroupExecutorBuilderImpl(
            TopExecutor<Ts...>& top,
            std::unique_ptr<GroupExecutor<Ts...>>&& g
        ) noexcept;

        template<class F>
        auto then(F&& f);

        template<class F>
        GroupExecutorBuilderImpl<1, hasThen, Ts...> on_exit(F&& f);
        GroupExecutorBuilderImpl<1, hasThen, Ts...> propagate_exit();

        operator R ();

    private:
        TopExecutor<Ts...>& top;
        std::unique_ptr<GroupExecutor<Ts...>> g;
    };

    template<bool setError, bool hasThen, class... Ts>
    struct [[nodiscard]] TopExecutorBuilderImpl
    {
        explicit TopExecutorBuilderImpl(TopExecutor<Ts...>& top) noexcept;

        template<class F>
        auto then(F&& f);

        template<class F>
        TopExecutorBuilderImpl<1, hasThen, Ts...> on_exit(F&& f);
        TopExecutorBuilderImpl<1, hasThen, Ts...> propagate_exit();

        // TODO ptr
        TopExecutor<Ts...>& done() { return this->top; }
        operator TopExecutor<Ts...>& () { return this->top; }

    private:
        TopExecutor<Ts...>& top;
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

    struct [[nodiscard]] TopExecutorBuilder_Concept
    {
        template<class... Ts>
        explicit TopExecutorBuilder_Concept(Ts&&...) noexcept;

        template<class F> TopExecutorBuilder_Concept then(F) { return *this; }
        template<class F> TopExecutorBuilder_Concept on_exit(F) { return *this; }
        TopExecutorBuilder_Concept propagate_exit();

        // TODO ptr
        TopExecutor<>& done();
        operator TopExecutor<>& ();
    };

    template<class...>
    using GroupExecutorBuilder = GroupExecutorBuilder_Concept;

    template<class...>
    using TopExecutorBuilder = TopExecutorBuilder_Concept;
#else
    template<class... Ts>
    using GroupExecutorBuilder = GroupExecutorBuilderImpl<0, 0, Ts...>;

    template<class... Ts>
    using TopExecutorBuilder = TopExecutorBuilderImpl<0, 0, Ts...>;
#endif
}

template<class... Ts>
struct Context
{
#ifdef IN_IDE_PARSER
    detail::GroupExecutorBuilder_Concept create_sub_executor();
#else
    auto create_sub_executor();
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

    int get_fd() const noexcept { return this->top.get_fd(); }
    void set_fd(int fd) const noexcept { this->top.set_fd(fd); }

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
        this->on_exit = [](ContextError<Ts...>, ExitR er, Ts&...){
            return static_cast<R>(er.status);
        };
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
    TopExecutor(int fd)
    : fd(fd)
    {}

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

    void set_fd(int fd) noexcept
    {
        assert(fd >= 0);
        this->fd = fd;
    }

    int get_fd() const noexcept
    {
        return this->fd;
    }

private:
    R _exec_next(GroupExecutor<Ts...>& top, Ts&... xs);
    R _exec(GroupExecutor<Ts...>& top, Ts&... xs);
    R _exec_exit(GroupExecutor<Ts...>& top, R r, Ts&... xs);
    R _exec_group_exit(GroupExecutor<Ts...>& top, R r, Ts&... xs);

    int fd;
    Error error = Error(NO_ERROR);
};


struct SharedDataBase
{
    int use_count;
    void* shared_ptr;
    void (*deleter) (SharedDataBase*) noexcept;
    SharedDataBase* next;

    void apply_deleter() noexcept
    {
        this->deleter(this);
    }

    bool alive() const noexcept
    {
        return bool(this->shared_ptr);
    }
};

template<class T>
struct SharedData : SharedDataBase
{
    template<class... Ts>
    SharedData(Ts&&... xs)
    : value(static_cast<Ts&&>(xs)...)
    {}

    T value;
};

template<class T> class SharedPtr;

template<class T>
struct SharedPtr
{
    SharedPtr() = default;

    SharedPtr(SharedPtr&& other) noexcept
    : p(std::exchange(other.p, nullptr))
    {
        this->p->shared_ptr = &this;
    }

    SharedPtr& operator=(SharedPtr&& other) noexcept
    {
        assert(other.p != this);
        this->reset();
        this->p = std::exchange(other.p, nullptr);
        this->p->shared_ptr = this;
        return *this;
    }

    ~SharedPtr()
    {
        this->reset();
    }

    void reset() noexcept
    {
        // if (this->p) {
        //     this->p->apply_deleter();
        // }
    }

    SharedData<T>* p;
};


struct SessionReactor
{
    using FirstTop = TopExecutor<>;
    using FirstTopPtr = SharedPtr<FirstTop>;

    #ifdef IN_IDE_PARSER
    detail::TopExecutorBuilder_Concept create_top_executor(int fd)
    #else
    auto create_top_executor(int fd)
    #endif
    {
        using D = SharedData<FirstTop>;
        D* data_ptr = new D{fd};
        data_ptr->use_count = 1;
        data_ptr->deleter = [](SharedDataBase* p) noexcept { delete static_cast<D*>(p); };
        data_ptr->shared_ptr = nullptr;
        data_ptr->next = this->node_executors;
        this->node_executors = data_ptr;
        return detail::TopExecutorBuilder<>{data_ptr->value};
    }

    ~SessionReactor()
    {
        SharedDataBase* p = this->node_executors;
        SharedDataBase* next;
        while (p) {
            next = p->next;
            p->apply_deleter();
            p = next;
        }
    }

    SharedDataBase* node_executors = nullptr;
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


template<bool setError, bool hasThen, class... Ts>
detail::GroupExecutorBuilderImpl<setError, hasThen, Ts...>::GroupExecutorBuilderImpl(
    TopExecutor<Ts...>& top, std::unique_ptr<GroupExecutor<Ts...>>&& g) noexcept
: top(top)
, g(std::move(g))
{}

template<bool setError, bool hasThen, class... Ts>
template<class F>
auto detail::GroupExecutorBuilderImpl<setError, hasThen, Ts...>::then(F&& f)
{
    this->g->then(static_cast<F&&>(f));
    return GroupExecutorBuilderImpl<
        setError, (hasThen == 1 ? hasThen : hasThen+1), Ts...>{
            this->top, std::move(this->g)};
}

template<bool setError, bool hasThen, class... Ts>
template<class F>
detail::GroupExecutorBuilderImpl<1, hasThen, Ts...>
detail::GroupExecutorBuilderImpl<setError, hasThen, Ts...>::on_exit(F&& f)
{
    static_assert(!setError, "on_error or propagate_exit is already used");
    this->g->set_on_exit(static_cast<F&&>(f));
    return GroupExecutorBuilderImpl<1, hasThen, Ts...>{this->top, std::move(this->g)};
}

template<bool setError, bool hasThen, class... Ts>
detail::GroupExecutorBuilderImpl<1, hasThen, Ts...>
detail::GroupExecutorBuilderImpl<setError, hasThen, Ts...>::propagate_exit()
{
    static_assert(!setError, "on_error or propagate_exit is already used");
    this->g->set_propagate_exit();
    return GroupExecutorBuilderImpl<1, hasThen, Ts...>{this->top, std::move(this->g)};
}

template<bool setError, bool hasThen, class... Ts>
detail::GroupExecutorBuilderImpl<setError, hasThen, Ts...>::operator R()
{
    static_assert(hasThen >= 1, "empty group");
    static_assert(setError, "missing builder.on_error(f) or builder.propagate_exit()");
    this->top.add_group_executor(std::move(this->g));
    return R::CreateGroup;
}


template<bool setError, bool hasThen, class... Ts>
detail::TopExecutorBuilderImpl<setError, hasThen, Ts...>::TopExecutorBuilderImpl(
    TopExecutor<Ts...>& top) noexcept
: top(top)
{}

template<bool setError, bool hasThen, class... Ts>
template<class F>
auto detail::TopExecutorBuilderImpl<setError, hasThen, Ts...>::then(F&& f)
{
    this->top.then(static_cast<F&&>(f));
    return TopExecutorBuilderImpl<
        setError, (hasThen == 1 ? hasThen : hasThen+1), Ts...>{
            this->top};
}

template<bool setError, bool hasThen, class... Ts>
template<class F>
detail::TopExecutorBuilderImpl<1, hasThen, Ts...>
detail::TopExecutorBuilderImpl<setError, hasThen, Ts...>::on_exit(F&& f)
{
    static_assert(!setError, "on_error or propagate_exit is already used");
    this->top.set_on_exit(static_cast<F&&>(f));
    return TopExecutorBuilderImpl<1, hasThen, Ts...>{this->top};
}

template<bool setError, bool hasThen, class... Ts>
detail::TopExecutorBuilderImpl<1, hasThen, Ts...>
detail::TopExecutorBuilderImpl<setError, hasThen, Ts...>::propagate_exit()
{
    static_assert(!setError, "on_error or propagate_exit is already used");
    this->top.set_propagate_exit();
    return TopExecutorBuilderImpl<1, hasThen, Ts...>{this->top};
}


template<class... Ts>
#ifdef IN_IDE_PARSER
detail::GroupExecutorBuilder_Concept Context<Ts...>::create_sub_executor()
#else
auto Context<Ts...>::create_sub_executor()
#endif
{
    return detail::GroupExecutorBuilder<Ts...>{
        this->top, std::make_unique<GroupExecutor<Ts...>>()};
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


template<class... Fs>
auto sequencer(Fs&&... fs)
{
    return [=, i = 0](auto ctx, auto&... xs) mutable {
        // TODO switch
        std::function<R(decltype(ctx), decltype(xs)...)> f[]{fs...};
        R r = f[i](ctx, xs...);
        return (r == R::Next && i < int(sizeof...(fs)-1)) ? ((void)++i, R::Substitute) : r;
    };
}

namespace jln2
{
    template<class... Ts> class TopExecutor;
    template<class... Ts> class GroupExecutor;
    template<class Tuple, class... Ts> struct GroupExecutorWithValues;

    enum class [[nodiscard]] R : char
    {
        Next,
        Terminate,
        ExitSuccess,
        ExitError,
        Exception,
        CreateGroup,
        NeedMoreData,
        SubstituteAction,
        Ready,
        CreateContinuation,
    };

    struct ExitR {
        enum Status : char {
            Error = char(R::ExitError),
            Success = char(R::Next),
            Exception = char(R::Exception),
            Terminate = char(R::Terminate),
        };

        Status status;
        ::Error& error;
    };

    namespace detail
    {
        template<bool HasAct, bool HasExit, class Top, class Group>
        struct [[nodiscard]] GroupExecutorBuilderImpl
        {
            explicit GroupExecutorBuilderImpl(Top& top, std::unique_ptr<Group>&& g) noexcept;

            template<class F>
            auto on_action(F&& f);

            template<class F>
            auto on_exit(F&& f);

            auto propagate_exit();

        private:
            Top& top;
            std::unique_ptr<Group> g;
        };

        template<bool HasAct, bool HasExit, class Top, class Group>
        struct [[nodiscard]] TopExecutorBuilderImpl
        {
            explicit TopExecutorBuilderImpl(Top& top, std::unique_ptr<Group>&& g) noexcept;

            template<class F>
            auto on_action(F&& f);

            template<class F>
            auto on_exit(F&& f);

            auto propagate_exit();

        private:
            Top& top;
            std::unique_ptr<Group> g;
        };

    #ifdef IN_IDE_PARSER
        struct /*[[nodiscard]]*/ GroupExecutorBuilder_Concept
        {
            template<class... Ts>
            explicit GroupExecutorBuilder_Concept(Ts&&...) noexcept;

            template<class F> GroupExecutorBuilder_Concept on_action(F) { return *this; }
            template<class F> GroupExecutorBuilder_Concept on_exit(F) { return *this; }
            GroupExecutorBuilder_Concept propagate_exit();
        };

        struct /*[[nodiscard]]*/ TopExecutorBuilder_Concept
        {
            template<class... Ts>
            explicit TopExecutorBuilder_Concept(Ts&&...) noexcept;

            template<class F> TopExecutorBuilder_Concept on_action(F) { return *this; }
            template<class F> TopExecutorBuilder_Concept on_exit(F) { return *this; }
            TopExecutorBuilder_Concept propagate_exit();
        };

        template<class Top, class Group>
        using GroupExecutorBuilder = GroupExecutorBuilder_Concept;

        template<class Top, class Group>
        using TopExecutorBuilder = TopExecutorBuilder_Concept;
    #else
        template<class Top, class Group>
        using GroupExecutorBuilder = GroupExecutorBuilderImpl<0, 0, Top, Group>;

        template<class Top, class Group>
        using TopExecutorBuilder = TopExecutorBuilderImpl<0, 0, Top, Group>;
    #endif
    }

    using jln::detail::decay_and_strip_t;

    enum class NextMode { ChildToNext, CreateContinuation, };

    template<class... Ts>
    struct Context
    {
        Context(TopExecutor<Ts...>& top, GroupExecutor<Ts...>& current_group) noexcept
        : top(top)
        , current_group(current_group)
        {}

        template<class... Us>
    #ifdef IN_IDE_PARSER
        detail::GroupExecutorBuilder_Concept create_sub_executor(Us&&...);
    #else
        auto create_sub_executor(Us&&...);
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

        int get_fd() const noexcept;
        void set_fd(int fd) noexcept;

    protected:
        TopExecutor<Ts...>& top;
        GroupExecutor<Ts...>& current_group;
    };

    template<class Tuple, class... Ts>
    struct ContextWithValues : Context<Ts...>
    {
        template<class F>
        R replace_action(F&& f);
    };

    template<class... Ts>
    struct ContextError
    {
        TopExecutor<Ts...>& top;
    };

    template<class... Ts>
    struct GroupExecutor
    {
        std::function<R(Context<Ts...>, Ts...)> on_action;
        std::function<R(ContextError<Ts...>, ExitR er, Ts...)> on_exit;
        NextMode next_mode = NextMode::ChildToNext;
        GroupExecutor* next;

        // TODO replace by deleter function
        virtual ~GroupExecutor() {}
    };

    template<class Tuple, class... Ts>
    struct GroupExecutorWithValues : GroupExecutor<Ts...>
    {
        using Base = GroupExecutor<Ts...>;

        template<class... Us>
        GroupExecutorWithValues(Us&&... xs)
        REDEMPTION_DIAGNOSTIC_PUSH
        REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wmissing-braces")
        : t{static_cast<Us&&>(xs)...}
        {}
        REDEMPTION_DIAGNOSTIC_POP

        template<class F>
        void on_action(F&& f)
        {
            Base::on_action = [f, this](Context<Ts...> ctx, Ts... xs) mutable -> R {
                return this->t.invoke(
                    f, ContextWithValues<Tuple, Ts...>{ctx}, static_cast<Ts&>(xs)...);
            };
        }

        template<class F>
        void on_exit(F&& f)
        {
            Base::on_exit = [f, this](ContextError<Ts...> ctx, ExitR er, Ts... xs) mutable -> R {
                return this->t.invoke(f, ctx, er, static_cast<Ts&>(xs)...);
            };
        }

        Tuple t;
    };

    template<class... Fs>
    auto sequencer(Fs&&... fs)
    {
        return [=, i = 0](auto ctx, auto&&... xs) mutable -> R {
            int nth = 0;
            R r = R::ExitError;
            ((nth++ == i ? (void)(
                r = fs(ctx, static_cast<decltype(xs)&&>(xs)...)
            ) : void()), ...);

            switch (r) {
                case R::Next:
                    return i < int(sizeof...(fs)-1) ? ((void)++i, R::Ready) : R::Next;
                case R::CreateGroup:
                    ++i;
                    return R::CreateContinuation;
                case R::SubstituteAction:
                    ++i;
                    return r;
                default:
                    return r;
            }
        };
    }

    template<class... Ts>
    struct TopExecutor
    {
        TopExecutor(int fd)
        : fd(fd)
        {}

        ~TopExecutor()
        {
            auto* p = this->group;
            while (p) {
                delete std::exchange(p, p->next);
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

        void add_group(std::unique_ptr<GroupExecutor<Ts...>>&& group)
        {
            group->next = this->group;
            this->group = group.release();
        }

        void sub_group(std::unique_ptr<GroupExecutor<Ts...>>&& group)
        {
            this->loaded_group = std::move(group);
        }

        bool exec(Ts&... xs)
        {
            try {
                R r = this->_exec(xs...);
                return r == R::NeedMoreData || r == R::Ready;
            }
            catch (Error const& e) {
                this->error = e;
                R const r = this->_exec_exit(R::Exception, xs...);
                if (r == R::Exception) {
                    throw this->error;
                }
                return r == R::NeedMoreData || r == R::Ready;
            }
        }

    private:
        R _exec(Ts&... xs)
        {
            R const r = this->group->on_action(Context<Ts...>{*this, *this->group}, xs...);
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
                // case R::SubstituteExit:
                // TODO
                case R::SubstituteAction:
                    this->group->on_action = std::move(this->loaded_group->on_action);
                    this->loaded_group.reset();
                    return R::NeedMoreData;
            }
            REDEMPTION_UNREACHABLE();
        }

        R _exec_exit(R r, Ts&... xs)
        {
            do {
                R const re = this->group->on_exit(
                    ContextError<Ts...>{*this},
                    ExitR{static_cast<ExitR::Status>(r), this->error},
                    xs...);
                NextMode next_mode;
                switch (re) {
                    case R::ExitSuccess:
                    case R::Next:
                        next_mode = this->group->next_mode;
                        delete std::exchange(this->group, this->group->next);
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
                        delete std::exchange(this->group, this->group->next);
                        r = re;
                        break;
                    case R::Ready:
                    case R::NeedMoreData:
                        return re;
                    case R::CreateGroup:
                    case R::SubstituteAction:
                        return R::Ready;
                    case R::CreateContinuation:
                        REDEMPTION_UNREACHABLE();
                }
            } while (this->group);

            return r;
        }

        int fd;
        GroupExecutor<Ts...>* group = nullptr;
        std::unique_ptr<GroupExecutor<Ts...>> loaded_group;
        Error error = Error(NO_ERROR);
    };

    struct Reactor
    {
        template<class... Ts>
        #ifdef IN_IDE_PARSER
        detail::TopExecutorBuilder_Concept create_top_executor(int fd, Ts&&... xs)
        #else
        auto create_top_executor(int fd, Ts&&... xs)
        #endif
        {
            using Top = TopExecutor<>;
            using Group = GroupExecutorWithValues<
                jln::detail::tuple<decay_and_strip_t<Ts>...>>;
            this->top = std::make_unique<Top>(fd);
            return detail::TopExecutorBuilder<Top, Group>{
                *this->top, std::make_unique<Group>(static_cast<Ts&&>(xs)...)};
        }

        std::unique_ptr<TopExecutor<>> top;
    };

    template<class... Ts>
    template<class... Us>
    #ifdef IN_IDE_PARSER
    detail::GroupExecutorBuilder_Concept Context<Ts...>::create_sub_executor(Us&&... xs)
    #else
    auto Context<Ts...>::create_sub_executor(Us&&... xs)
    #endif
    {
        using Top = TopExecutor<Ts...>;
        using Group = GroupExecutorWithValues<
            jln::detail::tuple<decay_and_strip_t<Us>...>, Ts...>;
        return detail::GroupExecutorBuilder<Top, Group>{
            this->top, std::make_unique<Group>(static_cast<Us&&>(xs)...)};
    }

    template<class... Ts>
    R Context<Ts...>::exception(Error const& e) noexcept
    {
        this->top.error = e;
        return R::Exception;
    }

    template<class... Ts>
    int Context<Ts...>::get_fd() const noexcept
    {
        return this->top.get_fd();
    }

    template<class... Ts>
    void Context<Ts...>::set_fd(int fd) noexcept
    {
        return this->top.set_fd(fd);
    }

    template<class Tuple, class... Ts>
    template<class F>
    R ContextWithValues<Tuple, Ts...>::replace_action(F&& f)
    {
        auto& group = static_cast<GroupExecutorWithValues<Tuple, Ts...>&>(this->current_group);
        auto g = std::make_unique<GroupExecutor<Ts...>>();
        // TODO same in GroupExecutorWithValues
        g->on_action = [f, &group](Context<Ts...> ctx, Ts... xs) mutable -> R {
            return group.t.invoke(
                f, ContextWithValues<Tuple, Ts...>{ctx}, static_cast<Ts&>(xs)...);
        };
        this->top.sub_group(std::move(g));
        return R::SubstituteAction;
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
        else {
            return detail::GroupExecutorBuilderImpl<HasAct, HasExit, Top, Group>{top, std::move(g)};
        }
    }

    template<bool HasAct, bool HasExit, class Top, class Group>
    template<class F>
    auto detail::GroupExecutorBuilderImpl<HasAct, HasExit, Top, Group>::on_action(F&& f)
    {
        static_assert(!HasAct, "on_action is already used");
        this->g->on_action(static_cast<F&&>(f));
        return select_group_result<1, HasExit, Top, Group>(this->top, std::move(this->g));
    }

    template<bool HasAct, bool HasExit, class Top, class Group>
    template<class F>
    auto detail::GroupExecutorBuilderImpl<HasAct, HasExit, Top, Group>::on_exit(F&& f)
    {
        static_assert(!HasExit, "on_exit or propagate_exit is already used");
        this->g->on_exit(static_cast<F&&>(f));
        return select_group_result<HasAct, 1, Top, Group>(this->top, std::move(this->g));
    }

    template<bool HasAct, bool HasExit, class Top, class Group>
    auto detail::GroupExecutorBuilderImpl<HasAct, HasExit, Top, Group>::propagate_exit()
    {
        return this->on_exit([](auto /*ctx*/, ExitR er, [[maybe_unused]] auto&&... xs){
            return static_cast<R>(er.status);
        });
    }


    template<bool HasAct, bool HasExit, class Top, class Group>
    detail::TopExecutorBuilderImpl<HasAct, HasExit, Top, Group>::TopExecutorBuilderImpl(
        Top& top, std::unique_ptr<Group>&& g) noexcept
    : top(top)
    , g(std::move(g))
    {}

    template<bool HasAct, bool HasExit, class Top, class Group>
    auto select_top_result(Top& top, std::unique_ptr<Group>&& g)
    {
        if constexpr (HasExit && HasAct) {
            top.add_group(std::move(g));
        }
        else {
            return detail::TopExecutorBuilderImpl<HasAct, HasExit, Top, Group>{top, std::move(g)};
        }
    }

    template<bool HasAct, bool HasExit, class Top, class Group>
    template<class F>
    auto detail::TopExecutorBuilderImpl<HasAct, HasExit, Top, Group>::on_action(F&& f)
    {
        static_assert(!HasAct, "on_action is already used");
        this->g->on_action(static_cast<F&&>(f));
        return select_top_result<1, HasExit, Top, Group>(this->top, std::move(this->g));
    }

    template<bool HasAct, bool HasExit, class Top, class Group>
    template<class F>
    auto detail::TopExecutorBuilderImpl<HasAct, HasExit, Top, Group>::on_exit(F&& f)
    {
        static_assert(!HasExit, "on_exit or propagate_exit is already used");
        this->g->on_exit(static_cast<F&&>(f));
        return select_top_result<HasAct, 1, Top, Group>(this->top, std::move(this->g));
    }

    template<bool HasAct, bool HasExit, class Top, class Group>
    auto detail::TopExecutorBuilderImpl<HasAct, HasExit, Top, Group>::propagate_exit()
    {
        return this->on_exit([](auto /*ctx*/, ExitR er, [[maybe_unused]] auto&&... xs){
            return static_cast<R>(er.status);
        });
    }
}


#include <iostream>
#include <sstream>
#include <stdexcept>

int main()
{
    std::stringbuf sbuf;
    auto* oldbuf = std::cout.rdbuf(&sbuf);

    SessionReactor session_reactor;

    using Context = ::Context<>;

    TopExecutor<>& e = session_reactor.create_top_executor(1)
    .propagate_exit()
    .then([](Context ctx){std::cout << "1\n"; return ctx.next(); })
    .then([](Context ctx){
        std::cout << "2\n";
        return ctx.create_sub_executor()
        .then([](Context ctx){std::cout << "2.1\n"; return ctx.next(); })
        .then([](Context ctx){std::cout << "2.2\n"; return ctx.next(); })
        .propagate_exit();

    })
    .then([](Context ctx){
        std::cout << "3\n";
        return ctx.create_sub_executor()
        .then([i = 1](Context ctx) {
            std::cout << "3.1  " << i << "\n";
            return ctx.create_sub_executor()
            .then([](Context ctx){std::cout << "3.1.1\n"; return ctx.next(); })
            .then([](Context ctx){std::cout << "3.1.2\n"; return ctx.next(); })
            .propagate_exit();
        })
        .propagate_exit();
    })
    .then([&](Context ctx){
        std::cout << "4\n";
        return ctx.create_sub_executor()
        .then([](Context ctx){std::cout << "4.1\n"; return ctx.next(); })
        .then([](Context ctx){std::cout << "4.2\n"; return ctx.exit_on_success(); })
        .then([](Context ctx){std::cout << "4.3\n"; return ctx.next(); })
        .then([](Context ctx){std::cout << "4.4\n"; return ctx.next(); })
        .propagate_exit();
    })
    .then([](Context ctx){
        std::cout << "5\n";
        // TODO return ctx.then(...).then(...)
        return ctx.replace_action([](Context ctx){
            std::cout << "5bis\n";
            return ctx.next();
        });
    })
    .then(sequencer(
        [](Context ctx){ std::cout << "6.1\n"; return ctx.next(); },
        [](Context ctx){ std::cout << "6.2\n"; return ctx.next(); }
    ))
    .then(sequencer(
        [](Context ctx){ std::cout << "7.1\n";
            return ctx.create_sub_executor()
            .then([](Context ctx){std::cout << "7.1.1\n"; return ctx.next(); })
            .then([](Context ctx){std::cout << "7.1.2\n"; return ctx.next(); })
            .then([](Context ctx){std::cout << "7.1.3\n"; return ctx.next(); })
            .then([](Context ctx){std::cout << "7.1.4\n"; return ctx.next(); })
            .propagate_exit();
        },
        [](Context ctx){ std::cout << "7.2\n"; return ctx.next(); }
    ))
    .then([](Context ctx){ std::cout << "8\n"; return ctx.next(); });


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

    std::cout.rdbuf(oldbuf);

    auto s = sbuf.str();
    std::cout << s;
    if (s != R"(.1
.2
.2.1
.2.2
.3
.3.1  1
.3.1.1
.3.1.2
.4
.4.1
.4.2
.5
.5bis
.6.1
.6.2
.7.1
.7.1.1
.7.1.2
.7.1.3
.7.1.4
.8
)")
    {
        throw std::runtime_error("output differ");
    }

    std::cout << "--------------\n";

    {
        std::stringbuf sbuf;
        auto* oldbuf = std::cout.rdbuf(&sbuf);
        std::cout << std::nounitbuf;

        // using Top = jln2::TopExecutor<>;
        // using Group = jln2::GroupExecutor<>;
        using Context = jln2::Context<>;

        jln2::Reactor reactor;

        reactor.create_top_executor(1)
        .on_action(jln2::sequencer(
            [](Context ctx){ std::cout << "1\n"; return ctx.next(); },
            [](Context ctx){
                std::cout << "2\n";
                return ctx.create_sub_executor()
                .on_action(jln2::sequencer(
                    [](Context ctx){std::cout << "2.1\n"; return ctx.next(); },
                    [](Context ctx){std::cout << "2.2\n"; return ctx.next(); }
                ))
                .propagate_exit();
            },
            [](Context ctx){
                std::cout << "3\n";
                return ctx.create_sub_executor()
                .on_action([i = 1](Context ctx) {
                    std::cout << "3.1  " << i << "\n";
                    return ctx.create_sub_executor()
                    .on_action(jln2::sequencer(
                        [](Context ctx){std::cout << "3.1.1\n"; return ctx.next(); },
                        [](Context ctx){std::cout << "3.1.2\n"; return ctx.next(); }
                    ))
                    .propagate_exit();
                })
                .propagate_exit();
            },
            [](Context ctx){
                std::cout << "4\n";
                return ctx.create_sub_executor()
                .on_action(jln2::sequencer(
                    [](Context ctx){std::cout << "4.1\n"; return ctx.next(); },
                    [](Context ctx){std::cout << "4.2\n"; return ctx.exit_on_success(); },
                    [](Context ctx){std::cout << "4.3\n"; return ctx.next(); },
                    [](Context ctx){std::cout << "4.4\n"; return ctx.next(); }
                ))
                .propagate_exit();
            },
            [](Context ctx){
                std::cout << "5\n";
                return ctx.create_sub_executor()
                .on_action(jln2::sequencer(
                    [](Context ctx){std::cout << "5.1\n"; return ctx.next(); },
                    [](auto ctx){
                        std::cout << "5.2\n";
                        return ctx.replace_action(
                            [](Context ctx){ std::cout << "5.2.1\n"; return ctx.next();
                        });
                    },
                    [](Context ctx){std::cout << "5.3\n"; return ctx.next(); }
                ))
                .propagate_exit();
            },
            jln2::sequencer(
                [](Context ctx){ std::cout << "6.1\n"; return ctx.next(); },
                [](Context ctx){ std::cout << "6.2\n"; return ctx.next(); }
            ),
            jln2::sequencer(
                [](Context ctx){ std::cout << "7.1\n";
                    return ctx.create_sub_executor(1)
                    .on_action(jln2::sequencer(
                        [](Context ctx, int){std::cout << "7.1.1\n"; return ctx.next(); },
                        [](Context ctx, int){std::cout << "7.1.2\n"; return ctx.next(); },
                        [](Context ctx, int){std::cout << "7.1.3\n"; return ctx.next(); },
                        [](Context ctx, int){std::cout << "7.1.4\n"; return ctx.next(); }
                    ))
                    .propagate_exit();
                },
                [](Context ctx){ std::cout << "7.2\n"; return ctx.next(); }
            ),
            [](Context ctx){ std::cout << "8\n"; return ctx.next(); }
        ))
        .propagate_exit();

        for (int i = 0; ; ++i) {
            if (i >= 30) {
                std::cout << "KO\n";
                break;
            }
            std::cout << ".";
            if (!reactor.top->exec()) {
                break;
            }
        }

        std::cout.rdbuf(oldbuf);

        auto s = sbuf.str();
        std::cout << s;
        if (s != R"(.1
.2
.2.1
.2.2
.3
.3.1  1
.3.1.1
.3.1.2
.4
.4.1
.4.2
.5
.5.1
.5.2
.5.2.1
.6.1
.6.2
.7.1
.7.1.1
.7.1.2
.7.1.3
.7.1.4
.7.2
.8
)")
        {
            throw std::runtime_error("output differ");
        }
    }
}
