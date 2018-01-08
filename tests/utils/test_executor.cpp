/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni, Meng Tan
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Unit test to rect object

*/

#define RED_TEST_MODULE TestExecutor
#include "system/redemption_unit_tests.hpp"

#include "utils/executor.hpp"

#include <iostream>

#define TRACE_n(n) std::cout << "\x1b[32m" << n << "\x1b[0m\n"
#define TRACE_II(x) std::cout << "\x1b[31m" #x "\x1b[0m\n"
#define TRACE_I(x) TRACE_II(x)
#define TRACE TRACE_I(__LINE__)

#include "core/RDP/tpdu_buffer.hpp"
#include "core/RDP/nego.hpp"

template<class Scheduler, class Ctx, class F, class... Fs>
struct SequenceCtx : Ctx
{
    template<class... Args>
    ExecutorResult next(Args&&... args)
    {
        return make_lambda<F>()(*this, static_cast<Args&&>(args)...);
    }

    ExecutorResult exit(ExitStatus status)
    {
        return (status == ExitStatus::Success) ? this->exit_on_success() : this->exit_on_error();
    }

    ExecutorResult exit_on_error()
    {
        return this->executor.result_exit_failure();
    }

    ExecutorResult exit_on_success()
    {
        return Ctx::next_action([](auto ctx, auto&&... args){
            return make_lambda<Scheduler>()(
                static_cast<SequenceCtx<Scheduler, decltype(ctx), Fs...>&>(ctx),
                static_cast<decltype(args)&&>(args)...
            );
        });
    }

    template<class FF, bool cond = 0>
    ExecutorResult next_action(FF)
    {
        static_assert(cond, "next_action must be used to last step");
        return detail::FriendExecutorResult::Nothing;
    }
};

template<class Scheduler, class Ctx, class F>
struct SequenceCtx<Scheduler, Ctx, F> : Ctx
{
    template<class... Args>
    ExecutorResult next(Args&&... args)
    {
        return make_lambda<F>()(static_cast<Ctx&>(*this), static_cast<Args&&>(args)...);
    }
};

#ifdef IN_IDE_PARSER
template<class Scheduler, class... Fs>
struct SequenceExecutor
{
    template<class F>
    SequenceExecutor then(F) &&;

    auto to_function() &&
    {
        return [](auto&&...) { return detail::FriendExecutorResult::Nothing; };
    }
};
#else
template<class Scheduler, class... Fs>
struct SequenceExecutor
{
    template<class F>
    CXX_WARN_UNUSED_RESULT
    SequenceExecutor<Scheduler, Fs..., F>
    then(F) &&
    {
        return {};
    }

    CXX_WARN_UNUSED_RESULT
    auto to_function() &&
    {
        return [](auto ctx, auto&&... args){
            return make_lambda<Scheduler>()(
                static_cast<SequenceCtx<Scheduler, decltype(ctx), Fs...>&>(ctx),
                static_cast<decltype(args)&&>(args)...
            );
        };
    }

    template<class Ctx, class... Args>
    auto operator()(Ctx ctx, Args&&... args)
    {
        return make_lambda<Scheduler>()(
            static_cast<SequenceCtx<Scheduler, decltype(ctx), Fs...>&>(ctx),
            static_cast<decltype(args)&&>(args)...
        );
    }
};
#endif


template<class Scheduler>
SequenceExecutor<Scheduler> sequence_executor(Scheduler)
{
    return {};
};

inline auto sequence_executor()
{
    return sequence_executor([](auto ctx, auto&&... args){
        return ctx.next(static_cast<decltype(args)&&>(args)...);
    });
};

RED_AUTO_TEST_CASE(TestExecutor)
{
//     enum class ModState : uint8_t {
//           MOD_RDP_NEGO
//         , MOD_RDP_BASIC_SETTINGS_EXCHANGE
//         , MOD_RDP_CHANNEL_CONNECTION_ATTACH_USER
//         , MOD_RDP_CHANNEL_JOIN_CONFIRME
//         , MOD_RDP_GET_LICENSE
//         , MOD_RDP_CONNECTED
//     };
//     TpduBuffer buf;
//     Transport trans;
//     URandom rand;
//     TimeObj timeobj;
//     std::string m;
//     RdpNego nego(false, trans, "", false, "", 'a', rand, timeobj, m, {});
//     struct ServerCert
//     {
//         const bool              store;
//         const ServerCertCheck   check;
//         std::stirng             path;
//         NullServerNotifier      notifier;
//     };
//
//     sequence_executor([](auto ctx, TpduBuffer& buf, Transport& trans, RdpNego& nego, ServerCert& cert){
//         buf.load_data(trans);
//         if (buf.next_pdu()) {
//             return ctx.next(nego, InStream(buf.current_pdu_buffer()), cert);
//         }
//         return ctx.retry();
//     })
//     .then([](auto ctx, RdpNego& nego, InStream x224_data, ServerCert& cert){
//         if (nego.recv_connection_confirm(
//             cert.store, cert.check, cert.notifier, cert.path.c_str(),
//             x224_data
//         )) {
//             return ctx.exit_on_success();
//         }
//         return ctx.retry();
//     })
//     .then([](auto ctx, RdpNego& nego, InStream x224_data, ServerCert& cert){
//         if (nego.recv_credssp(x224_data)) {
//             return ctx.exit_on_success();
//         }
//         return ctx.retry();
//     });
//
    Executor executor;
//     executor.initial_executor(std::ref(buf), std::ref(trans), std::ref(nego))
//         .on_action([](auto ctx, TpduBuffer& buf, Transport& trans, RdpNego& nego){
//             buf.load_data(trans);
//             if (buf.next_pdu()) {
//                 InStream x224_data(buf.current_pdu_buffer());
//                 nego.send_negotiation_request();
//                 nego.state = RdpNego::NEGO_STATE_NEGOCIATE;
//             }
//             TRACE;
//             return ctx.exit_on_success();
//         })
//         .on_timeout([](auto ctx) {
//             TRACE;
//             return ctx.exit_on_success();
//         })
//         .on_exit([](auto ctx, bool) {
//             TRACE;
//             return ctx.exit_on_success();
//         })
//     ;
//
//     executor.exec_all();


    executor.initial_executor(1, 2)
        .on_action([](auto ctx, int, int){
            TRACE;
            return ctx.sub_executor(0)
                .on_action([](auto ctx, int& i){
                    TRACE;
                    if (++i < 5) {
                        return ctx.retry();
                    }
                    return ctx.exit_on_success();
                })
                .on_timeout([](auto ctx, int){
                    TRACE;
                    return ctx.exit_on_success();
                })
                .on_exit([](auto ctx, bool, int){
                    TRACE;
                    return ctx.exit_on_success();
                })
            ;
        })
        .on_exit([](auto ctx, bool, int, int) {
            TRACE;
            return ctx.exit_on_success();
        })
        .on_timeout([](auto ctx, int, int) {
            TRACE;
            return ctx.exit_on_success();
        })
    ;

    executor.exec_all();


    executor.initial_executor(0)
        .on_action(
            sequence_executor([](auto ctx, int& n){
                ++n;
                TRACE_n(n);
                if (n & 1) {
                    return ctx.next();
                }
                return ctx.retry();
            })
            .then([](auto ctx){
                TRACE;
                return ctx.exit_on_success();
            })
            .then([](auto ctx){
                TRACE;
                return ctx.exit_on_success();
            })
        )
        .on_exit([](auto ctx, bool, int) {
            TRACE;
            return ctx.exit_on_success();
        })
        .on_timeout([](auto ctx, int) {
            TRACE;
            return ctx.exit_on_success();
        })
    ;

    executor.exec_all();
}
