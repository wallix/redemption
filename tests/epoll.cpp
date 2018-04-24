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
    template<class... Ts> class TopSharedPtr;
    class Reactor;

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

    struct ExitR
    {
        enum Status : char {
            Error = char(R::ExitError),
            Success = char(R::Next),
            Exception = char(R::Exception),
            Terminate = char(R::Terminate),
        };

        R to_result() const noexcept
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
            auto on_action(F&& f) &&;

            template<class F>
            auto on_exit(F&& f) &&;

            auto propagate_exit() &&;

        private:
            Top& top;
            std::unique_ptr<Group> g;
        };

        struct BuilderInit
        {
            enum E
            {
                None,
                Action = 1,
                Exit = 2,
                Timer = 4,
                Timeout = 8,
            };
        };

        template<BuilderInit::E, class InitCtx, class GroupPtr>
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
            auto on_action(F&& f) &&;

            template<class F>
            auto on_exit(F&& f) &&;

            template<class F>
            auto on_timeout(F&& f) &&;

            auto set_timeout(std::chrono::milliseconds ms) &&;

            auto propagate_exit() &&;

        private:
            InitCtx init_ctx;
            GroupPtr g;
        };

    #ifdef IN_IDE_PARSER
        struct Func
        {
            template<class F>
            Func(F) {}
        };

        struct /*[[nodiscard]]*/ GroupExecutorBuilder_Concept
        {
            template<class... Ts>
            explicit GroupExecutorBuilder_Concept(Ts&&...) noexcept;

            GroupExecutorBuilder_Concept on_action(Func) { return *this; }
            GroupExecutorBuilder_Concept on_exit(Func) { return *this; }
            GroupExecutorBuilder_Concept propagate_exit();
        };

        struct /*[[nodiscard]]*/ TopExecutorBuilder_Concept
        {
            template<class... Ts>
            explicit TopExecutorBuilder_Concept(Ts&&...) noexcept;

            TopExecutorBuilder_Concept set_timeout(std::chrono::milliseconds) { return *this; }
            TopExecutorBuilder_Concept on_timeout(Func) { return *this; }
            TopExecutorBuilder_Concept on_action(Func) { return *this; }
            TopExecutorBuilder_Concept on_exit(Func) { return *this; }
            TopExecutorBuilder_Concept propagate_exit();

            operator SharedPtr ();

            template<class... Ts> operator TopSharedPtr<Ts...> ();
        };

        template<class Top, class Group>
        using GroupExecutorBuilder = GroupExecutorBuilder_Concept;

        template<class Top, class Group>
        using TopExecutorBuilder = TopExecutorBuilder_Concept;
    #else
        template<class Top, class Group>
        using GroupExecutorBuilder = GroupExecutorBuilderImpl<0, 0, Top, Group>;

        template<class Top, class GroupPtr>
        using TopExecutorBuilder = TopExecutorBuilderImpl<BuilderInit::None, Top, GroupPtr>;
    #endif
    }

    using jln::detail::decay_and_strip_t;

    enum class NextMode { ChildToNext, CreateContinuation, };

    template<class... Ts>
    struct GroupContext
    {
        GroupContext(TopExecutor<Ts...>& top, GroupExecutor<Ts...>& current_group) noexcept
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
    struct TopContext : GroupContext<Ts...>
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
    struct TimerData
    {
        timeval tv {};
        std::chrono::milliseconds delay = std::chrono::milliseconds(-1);
        std::function<R(ContextTimer<Ts...>, Ts...)> timeout;
    };

    template<class... Ts>
    struct GroupExecutor
    {
        std::function<R(GroupContext<Ts...>, Ts...)> on_action;
        std::function<R(ContextExit<Ts...>, ExitR er, Ts...)> on_exit;
        NextMode next_mode = NextMode::ChildToNext;
        GroupExecutor* next;

        // TODO replace by deleter function
        virtual ~GroupExecutor() {}
    };

    template<class Tuple, class... Ts>
    // TODO specialization for GroupExecutorWithValues<tuple<>, Ts...>
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
            Base::on_action = [f, this](GroupContext<Ts...> ctx, Ts... xs) mutable -> R {
                return this->t.invoke(
                    f, TopContext<Tuple, Ts...>{ctx}, static_cast<Ts&>(xs)...);
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
        template<class... Us>
        GroupExecutorWithTimer(TimerData<Ts...>& timer_data, Us&&... xs)
        : GroupExecutorWithValues<Tuple, Ts...>(static_cast<Us&&>(xs)...)
        , timer_data(timer_data)
        {}

        void set_timeout(std::chrono::milliseconds ms)
        {
            this->timer_data.delay = ms;
        }

        template<class F>
        void on_timeout(F&& f)
        {
            this->timer_data.timeout = [f, this](ContextTimer<Ts...> ctx, Ts... xs) mutable -> R {
                return this->t.invoke(f, ctx, static_cast<Ts&>(xs)...);
            };
        }

    private:
        TimerData<Ts...>& timer_data;
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
        TopExecutor(Reactor& reactor, int fd)
        : fd(fd)
        , reactor(reactor)
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

        void set_timeout(std::chrono::milliseconds ms) noexcept;

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
                // while (r == R::Ready) {
                //     r = this->_exec(xs...);
                // }
                return r == R::NeedMoreData || r == R::Ready;
            }
            catch (Error const& e) {
                this->error = std::move(e);
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
            R const r = this->group->on_action(GroupContext<Ts...>{*this, *this->group}, xs...);
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
        Reactor& reactor;

    public: // TODO to private
        TimerData<Ts...> timer_data;

    private:
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

    protected:
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

    template<class... Ts>
    class TopSharedPtr : public SharedPtr
    {
        using Top = TopExecutor<Ts...>;
        using Data = SharedData<Top>;

    public:
        TopSharedPtr(Data* p) noexcept
        : SharedPtr(p)
        {}

        void update_timer(std::chrono::milliseconds ms) noexcept
        {
            this->timer().delay = ms;
        }

        void set_time(timeval const& tv) noexcept
        {
            this->timer().tv = tv;
        }

        void set_delay(std::chrono::milliseconds ms) noexcept
        {
            this->timer().delay = ms;
        }

    private:
        TimerData<Ts...>& timer() noexcept
        {
            return static_cast<Data*>(this->p)->value();
        }

        Top& value() noexcept
        {
            return static_cast<Data*>(this->p)->value();
        }
    };


#ifdef IN_IDE_PARSER
# define REDEMPTION_JLN2_CONCEPT(C) C
#else
# define REDEMPTION_JLN2_CONCEPT(C) auto
#endif

    template<class... Ts>
    class TopContainer
    {
        using Top = TopExecutor<Ts...>;
        using Group = GroupExecutor<Ts...>;
        using Data = SharedData<Top>;

    public:
        using Ptr = TopSharedPtr<Ts...>;

    private:
        struct InitContext
        {
            std::unique_ptr<Data, SharedDataDeleter> data_ptr;
            TopContainer& cont;

            Ptr terminate_init(std::unique_ptr<Group>&& group)
            {
                assert(this->data_ptr);
                this->data_ptr->value().add_group(std::move(group));
                SharedDataBase* data_ptr = this->data_ptr.release();
                data_ptr->next = std::exchange(this->cont.node_executors.next, data_ptr);
                data_ptr->shared_ptr = nullptr;
                return Ptr(static_cast<Data*>(data_ptr));
            }
        };

    public:
        template<class... Us>
        REDEMPTION_JLN2_CONCEPT(detail::TopExecutorBuilder_Concept)
        create_top_executor(Reactor& reactor, int fd, Us&&... xs)
        {
            using Group = GroupExecutorWithTimer<
                jln::detail::tuple<decay_and_strip_t<Us>...>, Ts...>;
            auto* data = new Data{reactor, fd};
            return detail::TopExecutorBuilder<InitContext, std::unique_ptr<Group>>{
                InitContext{std::unique_ptr<Data, SharedDataDeleter>(data), *this},
                std::make_unique<Group>(data->value().timer_data, static_cast<Us&&>(xs)...)};
        }

        TopContainer() noexcept
        {
            this->node_executors.next = nullptr;
        }

        ~TopContainer()
        {
            this->clear();
        }

        void clear()
        {
            while (this->node_executors.next) {
                SharedDataBase* p = &this->node_executors;
                SharedDataBase* next;
                SharedDataBase* node;
                while (p->next) {
                    node = p->next;
                    next = p->next->next;
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

        template<class F>
        static auto apply_f(F& f, SharedDataBase* node)
        {
            auto& top = static_cast<Data*>(node)->value();
            return f(top.get_fd(), top);
        }

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

        template<class Predicate>
        bool exec(Predicate&& predicate, Ts... xs)
        {
            SharedDataBase* node = &this->node_executors;
            while (node->next) {
                auto* cur = node->next;
                if (cur->has_value()) {
                    if (!apply_f(predicate, cur)
                     || static_cast<Data&>(*cur)->exec(static_cast<Ts&>(xs)...)) {
                        node = node->next;
                    }
                    else {
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

        SharedDataBase node_executors;
    };

    struct Reactor
    {
        TopContainer<> top;

        template<class... Us>
        REDEMPTION_JLN2_CONCEPT(detail::TopExecutorBuilder_Concept)
        create_top_executor(int fd, Us&&... xs)
        {
            return this->top.create_top_executor(*this, fd, static_cast<Us&&>(xs)...);
        }

        template<class Pred>
        bool top_exec(Pred&& pred)
        {
            return this->top.exec(pred);
        }

        void set_current_time(timeval const& now)
        {
            assert(now >= this->current_time);
            this->current_time = now;
        }

        timeval get_current_time() const noexcept
        {
            //assert((this->current_time.tv_sec /*> -1*/) && "current_time is uninitialized. Used set_current_time");
            return this->current_time;
        }

        timeval current_time {};
    };

    template<class... Ts>
    void TopExecutor<Ts...>::set_timeout(std::chrono::milliseconds ms) noexcept
    {
        this->timer_data.delay = ms;
        this->timer_data.tv = addusectimeval(
            this->timer_data.delay, this->reactor.get_current_time());
    }

    template<class... Ts>
    template<class... Us>
    REDEMPTION_JLN2_CONCEPT(detail::GroupExecutorBuilder_Concept)
    GroupContext<Ts...>::create_sub_executor(Us&&... xs)
    {
        using Top = TopExecutor<Ts...>;
        using Group = GroupExecutorWithValues<
            jln::detail::tuple<decay_and_strip_t<Us>...>, Ts...>;
        return detail::GroupExecutorBuilder<Top, Group>{
            this->top, std::make_unique<Group>(static_cast<Us&&>(xs)...)};
    }

    template<class... Ts>
    R GroupContext<Ts...>::exception(Error const& e) noexcept
    {
        this->top.error = e;
        return R::Exception;
    }

    template<class... Ts>
    int GroupContext<Ts...>::get_fd() const noexcept
    {
        return this->top.get_fd();
    }

    template<class... Ts>
    void GroupContext<Ts...>::set_fd(int fd) noexcept
    {
        return this->top.set_fd(fd);
    }

    template<class Tuple, class... Ts>
    template<class F>
    R TopContext<Tuple, Ts...>::replace_action(F&& f)
    {
        auto& group = static_cast<GroupExecutorWithValues<Tuple, Ts...>&>(this->current_group);
        auto g = std::make_unique<GroupExecutor<Ts...>>();
        // TODO same in GroupExecutorWithValues
        g->on_action = [f, &group](GroupContext<Ts...> ctx, Ts... xs) mutable -> R {
            return group.t.invoke(
                f, TopContext<Tuple, Ts...>{ctx}, static_cast<Ts&>(xs)...);
        };
        this->top.sub_group(std::move(g));
        return R::SubstituteAction;
    }

    template<class Tuple, class... Ts>
    template<class F>
    R TopContext<Tuple, Ts...>::replace_exit(F&& f)
    {
        auto& group = static_cast<GroupExecutorWithValues<Tuple, Ts...>&>(this->current_group);
        auto g = std::make_unique<GroupExecutor<Ts...>>();
        // TODO same in GroupExecutorWithValues
        g->on_exit = [f, &group](ContextExit<Ts...> ctx, ExitR er, Ts... xs) mutable -> R {
            return group.t.invoke(
                f, TopContext<Tuple, Ts...>{ctx}, er, static_cast<Ts&>(xs)...);
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
    auto detail::GroupExecutorBuilderImpl<HasAct, HasExit, Top, Group>::on_action(F&& f) &&
    {
        static_assert(!HasAct, "on_action is already used");
        this->g->on_action(static_cast<F&&>(f));
        return select_group_result<1, HasExit, Top, Group>(this->top, std::move(this->g));
    }

    template<bool HasAct, bool HasExit, class Top, class Group>
    template<class F>
    auto detail::GroupExecutorBuilderImpl<HasAct, HasExit, Top, Group>::on_exit(F&& f) &&
    {
        static_assert(!HasExit, "on_exit or propagate_exit is already used");
        this->g->on_exit(static_cast<F&&>(f));
        return select_group_result<HasAct, 1, Top, Group>(this->top, std::move(this->g));
    }

    template<bool HasAct, bool HasExit, class Top, class Group>
    auto detail::GroupExecutorBuilderImpl<HasAct, HasExit, Top, Group>::propagate_exit() &&
    {
        return std::move(*this).on_exit([](auto /*ctx*/, ExitR er, [[maybe_unused]] auto&&... xs){
            return static_cast<R>(er.status);
        });
    }


    template<detail::BuilderInit::E Has, class InitCtx, class GroupPtr>
    detail::TopExecutorBuilderImpl<Has, InitCtx, GroupPtr>::TopExecutorBuilderImpl(
        InitCtx&& init_ctx, GroupPtr&& g) noexcept
    : init_ctx(std::move(init_ctx))
    , g(std::move(g))
    {}

    template<int Has, class InitCtx, class GroupPtr>
    auto select_top_result(InitCtx& init_ctx, GroupPtr& g)
    {
        using E = detail::BuilderInit::E;
        if constexpr (Has == (E::Action | E::Exit | E::Timer | E::Timeout)) {
            return init_ctx.terminate_init(std::move(g));
        }
        else {
            return detail::TopExecutorBuilderImpl<E(Has), InitCtx, GroupPtr>{
                std::move(init_ctx), std::move(g)};
        }
    }

    template<detail::BuilderInit::E Has, class InitCtx, class GroupPtr>
    template<class F>
    auto detail::TopExecutorBuilderImpl<Has, InitCtx, GroupPtr>::on_action(F&& f) &&
    {
        static_assert(!(Has & detail::BuilderInit::Action), "on_action is already used");
        this->g->on_action(static_cast<F&&>(f));
        return select_top_result<Has | detail::BuilderInit::Action>(this->init_ctx, this->g);
    }

    template<detail::BuilderInit::E Has, class InitCtx, class GroupPtr>
    template<class F>
    auto detail::TopExecutorBuilderImpl<Has, InitCtx, GroupPtr>::on_exit(F&& f) &&
    {
        static_assert(!(Has & detail::BuilderInit::Exit), "on_exit or propagate_exit is already used");
        this->g->on_exit(static_cast<F&&>(f));
        return select_top_result<Has | detail::BuilderInit::Exit>(this->init_ctx, this->g);
    }

    template<detail::BuilderInit::E Has, class InitCtx, class GroupPtr>
    auto detail::TopExecutorBuilderImpl<Has, InitCtx, GroupPtr>::set_timeout(std::chrono::milliseconds ms) &&
    {
        static_assert(!(Has & detail::BuilderInit::Timeout), "set_timeout is already used");
        this->g->set_timeout(ms);
        return select_top_result<Has | detail::BuilderInit::Timeout>(this->init_ctx, this->g);
    }

    template<detail::BuilderInit::E Has, class InitCtx, class GroupPtr>
    template<class F>
    auto detail::TopExecutorBuilderImpl<Has, InitCtx, GroupPtr>::on_timeout(F&& f) &&
    {
        static_assert(!(Has & detail::BuilderInit::Timer), "on_timeout is already used");
        this->g->on_timeout(static_cast<F&&>(f));
        return select_top_result<Has | detail::BuilderInit::Timer>(this->init_ctx, this->g);
    }

    template<detail::BuilderInit::E Has, class InitCtx, class GroupPtr>
    auto detail::TopExecutorBuilderImpl<Has, InitCtx, GroupPtr>::propagate_exit() &&
    {
        return std::move(*this).on_exit([](auto /*ctx*/, ExitR er, [[maybe_unused]] auto&&... xs){
            return er.to_result();
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
    using Context = jln2::GroupContext<>;

    jln2::Reactor reactor;

    jln2::SharedPtr top = reactor.create_top_executor(1)
    .set_timeout({})
    .on_timeout([](auto ctx){ return ctx.ready(); })
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


    jln2::TopSharedPtr<> top2 = reactor.create_top_executor(1)
    .set_timeout({})
    .on_timeout([](auto ctx){ return ctx.ready(); })
    .on_action(jln2::sequencer(
        [](Context ctx){ std::cout << "+1.1\n"; return ctx.next(); },
        [](Context ctx){ std::cout << "+1.2\n"; return ctx.next(); },
        [](Context ctx){ std::cout << "+1.3\n"; return ctx.next(); }
    ))
    .propagate_exit();

    // top.reset();

//     std::stringbuf sbuf;
//     auto* oldbuf = std::cout.rdbuf(&sbuf);
//     std::cout << std::nounitbuf;
//
//     for (int i = 0; ; ++i) {
//         if (i >= 30) {
//             std::cout << "KO\n";
//             break;
//         }
//         std::cout << ".";
//         if (!reactor.top_exec([](auto, auto&){ return true; })) {
//             break;
//         }
//     }
//
//     std::cout.rdbuf(oldbuf);
//
//     auto s = sbuf.str();
//     std::cout << s << std::flush;
//     if (s != R"(.+1.1
// 1
// .+1.2
// 2
// .+1.3
// 2.1
// .2.2
// .3
// .3.1  1
// .3.1.1
// .3.1.2
// .4
// .4.1
// .4.2
// .5
// .5.1
// .5.2
// .5.2.1
// .6.1
// .6.2
// .7.1
// .7.1.1
// .7.1.2
// .7.1.3
// .7.1.4
// .7.2
// .8
// )")
//     {
//         throw std::runtime_error("output differ");
//     }

    reactor.top.clear();
    reactor.top.clear();
}
