#include <functional>
#include <memory>
#include <cassert>
#include <exception>
#include <chrono>
#include "cxx/diagnostic.hpp"
#include "utils/executor.hpp"
#include "core/error.hpp"

#include <iostream>

namespace jln2
{
    template<class... Ts> class TopExecutor;
    template<class... Ts> class GroupExecutor;
    template<class Tuple, class... Ts> struct GroupExecutorWithValues;
    template<class T> class SharedData;
    class SharedPtr;

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
        SubstituteExit,
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

    enum class ExitStatus : bool {
        Error,
        Success,
    };

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
            auto on_action(F&& f);

            template<class F>
            auto on_exit(F&& f);

            auto propagate_exit();

        private:
            Top& top;
            std::unique_ptr<Group> g;
        };

        template<bool HasAct, bool HasExit, bool HasTimer, class InitCtx, class GroupPtr>
        struct [[nodiscard]] TopExecutorBuilderImpl
        {
            explicit TopExecutorBuilderImpl(InitCtx&& init_ctx, GroupPtr&& g) noexcept;

#ifndef NDEBUG
            ~TopExecutorBuilderImpl()
            {
                assert(!this->g);
            }
#endif

            template<class F>
            auto on_action(F&& f);

            template<class F>
            auto on_exit(F&& f);

            template<class F>
            auto on_timeout(std::chrono::milliseconds ms, F&& f);

            auto propagate_exit();

        private:
            InitCtx init_ctx;
            GroupPtr g;
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

            template<class F> TopExecutorBuilder_Concept on_timeout(std::chrono::milliseconds ms, F) { return *this; }
            template<class F> TopExecutorBuilder_Concept on_action(F) { return *this; }
            template<class F> TopExecutorBuilder_Concept on_exit(F) { return *this; }
            TopExecutorBuilder_Concept propagate_exit();

            operator SharedPtr ();
        };

        template<class Top, class Group>
        using GroupExecutorBuilder = GroupExecutorBuilder_Concept;

        template<class Top, class Group>
        using TopExecutorBuilder = TopExecutorBuilder_Concept;
    #else
        template<class Top, class Group>
        using GroupExecutorBuilder = GroupExecutorBuilderImpl<0, 0, Top, Group>;

        template<class Top, class GroupPtr>
        using TopExecutorBuilder = TopExecutorBuilderImpl<0, 0, 0, Top, GroupPtr>;
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

        template<class F>
        R replace_exit(F&& f);
    };

    template<class... Ts>
    struct ContextExit
    {
        TopExecutor<Ts...>& top;
        GroupExecutor<Ts...>& current_group;
    };

    template<class... Ts>
    struct ContextTimer
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

        TopExecutor<Ts...>& top;
        GroupExecutor<Ts...>& current_group;
    };

    template<class... Ts>
    struct GroupExecutor
    {
        std::function<R(Context<Ts...>, Ts...)> on_action;
        std::function<R(ContextExit<Ts...>, ExitR er, Ts...)> on_exit;
        NextMode next_mode = NextMode::ChildToNext;
        GroupExecutor* next;

        // TODO replace by deleter function
        virtual ~GroupExecutor() {}
    };

    template<class Tuple, class... Ts>
    struct GroupExecutorWithValues : GroupExecutor<Ts...>
    {
        using Base = GroupExecutor<Ts...>;

        REDEMPTION_DIAGNOSTIC_PUSH
        REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wmissing-braces")
        template<class... Us>
        GroupExecutorWithValues(Us&&... xs)
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
            Base::on_exit = [f, this](ContextExit<Ts...> ctx, ExitR er, Ts... xs) mutable -> R {
                return this->t.invoke(f, ctx, er, static_cast<Ts&>(xs)...);
            };
        }

        Tuple t;
    };

    template<class Tuple, class... Ts>
    struct GroupExecutorWithTimer : GroupExecutorWithValues<Tuple, Ts...>
    {
        template<class F>
        void on_timeout(std::chrono::milliseconds ms, F&& f)
        {
            this->timeout_ = [f, this](ContextTimer<Ts...> ctx, Ts... xs) mutable -> R {
                return this->t.invoke(f, ctx, static_cast<Ts&>(xs)...);
            };
            this->max_delay = ms;
        }

    private:
        std::function<R(ContextTimer<Ts...>, Ts...)> timeout_;
        std::chrono::milliseconds max_delay;
    };

    template<class... Fs>
    auto sequencer(Fs&&... fs)
    {
        return [=, i = 0](auto ctx, auto&&... xs) mutable -> R {
            // TODO optimise switch
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
                case R::SubstituteExit:
                    this->group->on_exit = std::move(this->loaded_group->on_exit);
                    this->loaded_group.reset();
                    return R::NeedMoreData;
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
                    ContextExit<Ts...>{*this, *this->group},
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
                    case R::SubstituteExit:
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

    struct SharedDataBase
    {
        enum class FreeCat { Value, Self, };
        SharedPtr* shared_ptr;
        void (*deleter) (SharedDataBase*, FreeCat) noexcept;
        SharedDataBase* next;

        void free_value() noexcept
        {
            this->deleter(this, FreeCat::Value);
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
            assert(other.p != this->p);
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
            if (this->p) {
                this->p->shared_ptr = nullptr;
                this->p->free_value();
                this->p = nullptr;
            }
        }

    private:
        friend class SharedDataBase;

        SharedDataBase* release() noexcept
        {
            return std::exchange(this->p, nullptr);
        }

        SharedDataBase* p;
    };

    inline void SharedDataBase::release_shared_ptr() noexcept
    {
        this->shared_ptr->release();
        this->shared_ptr = nullptr;
    }

    template<class T>
    struct SharedData : SharedDataBase
    {
        template<class... Ts>
        SharedData(Ts&&... xs)
        : u(static_cast<Ts&&>(xs)...)
        {
            this->deleter = [](SharedDataBase* p, FreeCat cat) noexcept {
                auto* self = static_cast<SharedData*>(p);
                switch (cat) {
                    case FreeCat::Value:
                        self->release_shared_ptr();
                        self->u.value.~T();
                        break;
                    case FreeCat::Self:
                        delete self;
                        break;
                }
            };
        }

        T* operator->() { return &this->u.value; }
        T& operator*() { return this->u.value; }

        T& value() { return this->u.value; }

    private:
        union U{
            T value;

            template<class... Ts>
            U(Ts&&... xs) : value(static_cast<Ts&&>(xs)...){}
            ~U() { /* removed by this->deleter */}
        } u;
    };

    struct SharedDataDeleter
    {
        void operator()(SharedDataBase* p) const noexcept
        {
            p->free_value();
            p->delete_self();
        }
    };

    struct Reactor
    {
        using Top = TopExecutor<>;

        template<class... Ts>
        struct TopInitContext
        {
            using Top = TopExecutor<Ts...>;
            using Group = GroupExecutor<Ts...>;

            std::unique_ptr<SharedData<Top>, SharedDataDeleter> data_ptr;
            Reactor& reactor;

            SharedPtr terminate_init(std::unique_ptr<Group>&& group)
            {
                assert(this->data_ptr);
                this->data_ptr->value().add_group(std::move(group));
                SharedDataBase* data_ptr = this->data_ptr.release();
                data_ptr->next = std::exchange(this->reactor.node_executors.next, data_ptr);
                data_ptr->shared_ptr = nullptr;
                return SharedPtr(data_ptr);
            }
        };

        template<class... Ts>
        #ifdef IN_IDE_PARSER
        detail::TopExecutorBuilder_Concept create_top_executor(int fd, Ts&&... xs)
        #else
        auto create_top_executor(int fd, Ts&&... xs)
        #endif
        {
            using Group = GroupExecutorWithTimer<
                jln::detail::tuple<decay_and_strip_t<Ts>...>>;
            using InitCtx = TopInitContext<>;
            using Data = SharedData<Top>;
            return detail::TopExecutorBuilder<InitCtx, std::unique_ptr<Group>>{
                InitCtx{std::unique_ptr<Data, SharedDataDeleter>(new Data{fd}), *this},
                std::make_unique<Group>(static_cast<Ts&&>(xs)...)};
        }

        Reactor() noexcept
        {
            this->node_executors.next = nullptr;
        }

        ~Reactor()
        {
            while (this->node_executors.next) {
                SharedDataBase* p = this->node_executors.next;
                SharedDataBase* next;
                while (p) {
                    next = p->next;
                    if (p->has_value()) {
                        p->free_value();
                    }
                    else {
                        p->delete_self();
                    }
                    p = next;
                }
            }
        }

        Top& top() noexcept
        {
            return *static_cast<SharedData<Top>&>(*this->node_executors.next);
        }

        bool exec_top()
        {
            auto* node = &this->node_executors;
            while (node->next) {
                auto* cur = node->next;
                if (cur->shared_ptr) {
                    bool const r = static_cast<SharedData<Top>&>(*cur)->exec();
                    if (!r) {
                        node->next = cur->next;
                        cur->free_value();
                        cur->delete_self();
                    }
                    else {
                        node = node->next;
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

    template<class Tuple, class... Ts>
    template<class F>
    R ContextWithValues<Tuple, Ts...>::replace_exit(F&& f)
    {
        auto& group = static_cast<GroupExecutorWithValues<Tuple, Ts...>&>(this->current_group);
        auto g = std::make_unique<GroupExecutor<Ts...>>();
        // TODO same in GroupExecutorWithValues
        g->on_exit = [f, &group](ContextExit<Ts...> ctx, ExitR er, Ts... xs) mutable -> R {
            return group.t.invoke(
                f, ContextWithValues<Tuple, Ts...>{ctx}, er, static_cast<Ts&>(xs)...);
        };
        this->top.sub_group(std::move(g));
        return R::SubstituteAction;
    }

    template<class... Ts>
    R ContextTimer<Ts...>::exception(Error const& e) noexcept
    {
        this->top.error = e;
        return R::Exception;
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


    template<bool HasAct, bool HasExit, bool HasTimer, class InitCtx, class GroupPtr>
    detail::TopExecutorBuilderImpl<HasAct, HasExit, HasTimer, InitCtx, GroupPtr>::TopExecutorBuilderImpl(InitCtx&& init_ctx, GroupPtr&& g) noexcept
    : init_ctx(std::move(init_ctx))
    , g(std::move(g))
    {}

    template<bool HasAct, bool HasExit, bool HasTimer, class InitCtx, class GroupPtr>
    auto select_top_result(InitCtx& init_ctx, GroupPtr& g)
    {
        if constexpr (HasExit && HasAct && HasTimer) {
            return init_ctx.terminate_init(std::move(g));
        }
        else {
            return detail::TopExecutorBuilderImpl<HasAct, HasExit, HasTimer, InitCtx, GroupPtr>{
                std::move(init_ctx), std::move(g)};
        }
    }

    template<bool HasAct, bool HasExit, bool HasTimer, class InitCtx, class GroupPtr>
    template<class F>
    auto detail::TopExecutorBuilderImpl<HasAct, HasExit, HasTimer, InitCtx, GroupPtr>::on_action(F&& f)
    {
        static_assert(!HasAct, "on_action is already used");
        this->g->on_action(static_cast<F&&>(f));
        return select_top_result<1, HasExit, HasTimer, InitCtx, GroupPtr>(this->init_ctx, this->g);
    }

    template<bool HasAct, bool HasExit, bool HasTimer, class InitCtx, class GroupPtr>
    template<class F>
    auto detail::TopExecutorBuilderImpl<HasAct, HasExit, HasTimer, InitCtx, GroupPtr>::on_exit(F&& f)
    {
        static_assert(!HasExit, "on_exit or propagate_exit is already used");
        this->g->on_exit(static_cast<F&&>(f));
        return select_top_result<HasAct, 1, HasTimer, InitCtx, GroupPtr>(this->init_ctx, this->g);
    }

    template<bool HasAct, bool HasExit, bool HasTimer, class InitCtx, class GroupPtr>
    template<class F>
    auto detail::TopExecutorBuilderImpl<HasAct, HasExit, HasTimer, InitCtx, GroupPtr>::on_timeout(
        std::chrono::milliseconds ms, F&& f)
    {
        static_assert(!HasExit, "on_timeout is already used");
        this->g->on_timeout(ms, static_cast<F&&>(f));
        return select_top_result<HasAct, HasExit, 1, InitCtx, GroupPtr>(this->init_ctx, this->g);
    }

    template<bool HasAct, bool HasExit, bool HasTimer, class InitCtx, class GroupPtr>
    auto detail::TopExecutorBuilderImpl<HasAct, HasExit, HasTimer, InitCtx, GroupPtr>::propagate_exit()
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
    // using Top = jln2::TopExecutor<>;
    // using Group = jln2::GroupExecutor<>;
    using Context = jln2::Context<>;

    jln2::Reactor reactor;

    jln2::SharedPtr top = reactor.create_top_executor(1)
    .on_timeout({}, [](auto ctx){ return ctx.ready(); })
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
                return ctx.create_sub_executor(0)
                .on_action(jln2::sequencer(
                    [](Context ctx, int& i){std::cout << "7.1." << ++i << "\n"; return ctx.next(); },
                    [](Context ctx, int& i){std::cout << "7.1." << ++i << "\n"; return ctx.next(); },
                    [](Context ctx, int& i){std::cout << "7.1." << ++i << "\n"; return ctx.next(); },
                    [](Context ctx, int& i){std::cout << "7.1." << ++i << "\n"; return ctx.next(); }
                ))
                .propagate_exit();
            },
            [](Context ctx){ std::cout << "7.2\n"; return ctx.next(); }
        ),
        [](Context ctx){ std::cout << "8\n"; return ctx.next(); }
    ))
    .propagate_exit();

    // top.reset();

    std::stringbuf sbuf;
    auto* oldbuf = std::cout.rdbuf(&sbuf);
    std::cout << std::nounitbuf;

    for (int i = 0; ; ++i) {
        if (i >= 30) {
            std::cout << "KO\n";
            break;
        }
        std::cout << ".";
        if (!reactor.exec_top()) {
            break;
        }
    }

    std::cout.rdbuf(oldbuf);

    auto s = sbuf.str();
    std::cout << s << std::flush;
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
