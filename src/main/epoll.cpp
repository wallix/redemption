#include "utils/executor.hpp"
#include <iostream>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <sys/epoll.h>
#include <unistd.h>

struct CheckError
{
    int i;
    CheckError(int i) : i(i)
    {
        if (i == -1) {
            perror(nullptr);
            std::abort();
        }
    }
    operator int () const { return i; }
};

constexpr struct
{
    int operator | (int i) const
    {
        return CheckError(i);
    }
} E {};

int main()
{
    using namespace std::chrono_literals;

    struct Buffer
    {
        char data_[5];
        int size_ = 0;

        Buffer() = default;
        Buffer(Buffer const&) = delete;
        Buffer& operator=(Buffer const&) = delete;

        void read()
        {
            this->size_ += E | ::read(0, this->data_ + this->size_, sizeof(this->data_) - this->size_);
        }

        void reset()
        {
            this->size_ = 0;
        }

        char* data() noexcept
        {
            return this->data_;
        }

        int size() const noexcept
        {
            return this->size_;
        }
    };

    int const epfd = E | epoll_create(1);
    Reactor<Buffer&> reactor;

    int fd = 0;

    auto& top_executor = reactor.create_executor(fd)
        .on_action([](auto ctx, Buffer& buf){
            std::cout << "--> read" << std::endl;
            if (buf.size() < 3)
            {
                return ctx.need_more_data();
            }
            else if (!strncmp(buf.data(), "qui", 3))
            {
                return ctx.terminate();
            }
            else if (!strncmp(buf.data(), "msg", 3))
            {
                std::cout << "--> msg" << std::endl;
                if (buf.size() > 3) {
                    std::cout.write(buf.data() + 3, buf.size() - 3) << std::endl;
                }
                return ctx.exec_sub_executor()
                .on_action([](auto ctx, Buffer& buf){
                    std::cout.write(buf.data(), buf.size()) << std::endl;
                    for (int i = 0; i < buf.size(); ++i) {
                        if (buf.data()[i] == ';') {
                            return ctx.exit_on_success();
                        }
                    }
                    return ctx.need_more_data();
                })
                .on_exit([](auto ctx, ExecutorError error, Buffer& /*buf*/){
                    std::cout << "exit msg: " << int(error) << std::endl;
                    return ctx.exit_on_success();
                })
                .exec_action(buf);
            }
            else if (!strncmp(buf.data(), "tim", 3))
            {
                std::cout << "--> tim" << std::endl;
                auto timer = ctx.create_timer(0)
                    .on_action(1s, [](auto ctx, Buffer& /*buf*/, int& i){
                        std::cout << "timer " << ++i << std::endl;
                        return ctx.retry_until(std::chrono::seconds(i/3));
                    });
                return ctx.create_sub_executor(std::move(timer))
                    .on_action([](auto ctx, Buffer& /*buf*/, auto&&...){
                        return ctx.exit_on_success();
                    })
                    .on_exit([](auto ctx, ExecutorError /*error*/, Buffer& /*buf*/, auto&&...){
                        return ctx.exit_on_success();
                    });
            }
            else if (!strncmp(buf.data(), "gam", 3))
            {
                std::cout << "--> gam" << std::endl;
                buf.reset();
                auto timer = ctx.create_timer(std::ref(ctx.get_basic_executor()))
                .on_action(2s, [](auto ctx, auto& buf, auto& executor){
                    std::cout << "timer" << std::endl;
                    (void)ctx.detach_timer();
                    executor.exit_with(ExecutorError::ExternalExit, buf);
                    return ctx.terminate();
                });
                return ctx.create_sub_executor(std::move(timer))
                .on_action([](auto ctx, auto& /*buf*/, auto& /*timer*/){
                    // timer.disable(ExecutorResult::ExitFailure);
                    std::cout << "good" << std::endl;
                    return ctx.exit_on_success();
                })
                .on_exit([](auto ctx, ExecutorError error, auto& /*buf*/, auto& /*timer*/){
                    std::cout << "exit gam: " << int(error) << std::endl;
                    return ctx.exit_on_success();
                });
            }
//             else if (!strncmp(buf.data(), "gam", 3))
//             {
//                 std::cout << "--> gam" << std::endl;
//                 buf.reset();
//                 auto timer = ctx.create_timed_executor()
//                 .on_timer(2s, [](auto ctx){
//                     std::cout << "timer" << std::endl;
//                     return ctx.exit_on_success();
//                 })
//                 .on_action([](auto ctx){
//                     // timer.disable(ExecutorResult::ExitFailure);
//                     std::cout << "good" << std::endl;
//                     return ctx.exit_on_success();
//                 })
//                 .on_exit([](auto ctx, ExecutorError error){
//                     std::cout << "exit gam: " << int(error) << std::endl;
//                     return ctx.exit_on_success();
//                 });
//             }
            else if (!strncmp(buf.data(), "ext", 3))
            {
                std::cout << "--> ext" << std::endl;
                buf.reset();
                return ctx.exit_on_success();
            }
            else
            {
                buf.reset();
                std::cout << "Unknow " << std::endl;
                return ctx.need_more_data();
            }
        })
        .on_exit([](auto ctx, ExecutorError error, Buffer& /*buf*/){
            std::cout << "error: " << int(error) << std::endl;
            return ctx.need_more_data();
        })
        .on_timeout(3s, [](auto ctx, Buffer& /*buf*/){
            std::cout << "Timeout" << std::endl;
            return ctx.need_more_data();
        })
        .base();

    epoll_event event { EPOLLIN, {} };
    event.data.ptr = &top_executor;
    E | epoll_ctl(epfd, EPOLL_CTL_ADD, 0, &event);

    Buffer buffer;

    while (reactor.executors.xs.size())
    {
        epoll_event events[2] {};
        auto const ms = reactor.timers.get_next_timeout().count();
        //std::cout << "timeout: " << ms << " ms" << std::endl;
        int count = E | epoll_wait(epfd, events, 2, ms);
        if (count) {
            buffer.read();
            if (!buffer.size()) {
                break;
            }
            for (int i = 0; i < count; ++i) {
                auto& executor = *static_cast<BasicExecutor<Buffer&>*>(events[i].data.ptr);
                if (!executor.exec(buffer)) {
                    reactor.executors.remove(executor);
                }
            }
        }
        else if (count < 0) {
            std::cout << "count: " << count << std::endl;
            break;
        }
        else {
            (void)reactor.timers.exec_timeout(buffer);
        }
    }
}
