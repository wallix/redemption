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

// #define TRACE_n(n) std::cout << "\x1b[32m" << n << "\x1b[0m\n"
#define TRACE_II(x) std::cout << "\x1b[31m" #x "\x1b[0m\n"
#define TRACE_I(x) TRACE_II(x)
#define TRACE TRACE_I(__LINE__)

#include "core/RDP/tpdu_buffer.hpp"
#include "core/RDP/nego.hpp"
#include "core/server_notifier_api.hpp"
#include "utils/difftimeval.hpp"
#include "utils/sugar/strutils.hpp"
#include "test_only/transport/test_transport.hpp"
#include "test_only/lcg_random.hpp"

// template<char... cs>
// struct string_c{};
//
// template<class...>
// struct list
// {};
//
// REDEMPTION_DIAGNOSTIC_PUSH
// REDEMPTION_DIAGNOSTIC_CLANG_IGNORE("-Wgnu-string-literal-operator-template")
// template<class C, C... cs>
// string_c<cs...> operator ""_cs ()
// { return {}; }
// REDEMPTION_DIAGNOSTIC_POP
//
// template<class S, class F>
// struct NamedFunction
// {
//     template<class... Args>
//     auto operator()(Args&&... args)
//     {
//         return make_lambda<F>()(static_cast<decltype(args)&&>(args)...);
//     }
// };
//
//
// template<class FL, class Scheduler, class Ctx, class F, class... Fs>
// struct SequenceCtx;
//
// template<class S, class L>
// struct JumpSequenceCtx;
//
// template<class S, class T, class... Ts>
// struct JumpSequenceCtx<S, list<T, Ts...>>
// : JumpSequenceCtx<S, list<Ts...>>
// {};
//
// template<class S, class T, class... Ts>
// struct JumpSequenceCtx<S, list<NamedFunction<S, T>, Ts...>>
// {
//     template<class... Us>
//     using make = SequenceCtx<Us..., NamedFunction<S, T>, Ts...>;
// };
//
//
// template<bool isRetryState, class FL, class Scheduler, class Ctx, class F, class... Fs>
// struct SequenceThenCtx;
//
// template<class FL, class Scheduler, class Ctx, class F, class... Fs>
// struct SequenceCtx : Ctx
// {
//     template<class... Args>
//     static ExecutorResult next(Args&&... args)
//     {
//         auto& ctx = static_cast<SequenceThenCtx<0, FL, Scheduler, Ctx, F, Fs...>&>(static_cast<Ctx&>(*this));
//         return make_lambda<F>()(ctx, static_cast<Args&&>(args)...);
//     }
//
//     template<class NewScheduler>
//     static ExecutorResult next_action(NewScheduler)
//     {
//         return Ctx::next_action([](auto ctx, auto&&... args){
//             auto& new_ctx = static_cast<Ctx&>(ctx);
//             return make_lambda<NewScheduler>()(
//                 static_cast<SequenceCtx<FL, NewScheduler, Ctx, F, Fs...>&>(new_ctx),
//                 static_cast<decltype(args)&&>(args)...
//             );
//         });
//     }
// };
//
// template<bool isRetryState, class FL, class Scheduler, class Ctx, class F, class... Fs>
// struct SequenceThenCtx : Ctx
// {
//     static ExecutorResult exit(ExitStatus status)
//     {
//         return (status == ExitStatus::Success) ? this->exit_on_success() : this->exit_on_error();
//     }
//
//     static ExecutorResult exit_on_error()
//     {
//         return this->executor.result_exit_failure();
//     }
//
//     static ExecutorResult exit_on_success()
//     {
//         if constexpr (bool(sizeof...(Fs))) {
//             return Ctx::next_action([](auto ctx, auto&&... args){
//                 auto& new_ctx = static_cast<Ctx&>(ctx);
//                 return make_lambda<Scheduler>()(
//                     static_cast<SequenceCtx<FL, Scheduler, Ctx, Fs...>&>(new_ctx),
//                     static_cast<decltype(args)&&>(args)...
//                 );
//             });
//         }
//         else {
//             return Ctx::exit_on_success();
//         }
//     }
//
//     template<class FF>
//     static ExecutorResult next_action(FF)
//     {
//         return Ctx::next_action([](auto ctx, auto&&... args){
//             auto& new_ctx = static_cast<Ctx&>(ctx);
//             return make_lambda<Scheduler>()(
//                 static_cast<SequenceCtx<FL, Scheduler, Ctx, FF, Fs...>&>(new_ctx),
//                 static_cast<decltype(args)&&>(args)...
//             );
//         });
//     }
//
//     static ExecutorResult retry()
//     {
//         if constexpr (isRetryState) {
//             return Ctx::retry();
//         }
//         else {
//             return Ctx::next_action([](auto ctx, auto&&... args){
//                 auto& new_ctx = static_cast<SequenceThenCtx<1, FL, Scheduler, Ctx, F, Fs...>&>(static_cast<Ctx&>(ctx));
//                 return make_lambda<F>()(new_ctx, static_cast<decltype(args)&&>(args)...);
//             });
//         }
//     }
//
//     static ExecutorResult exit_success_sequence()
//     {
//         return this->executor.result_exit_success();
//     }
//
//     template<char... cs, class... Args>
//     static ExecutorResult fallthrough(Args&&... args)
//     {
//         static_assert(sizeof...(Fs), "is last callback");
//
//         auto& new_ctx = static_cast<Ctx&>(*this);
//         return static_cast<SequenceCtx<FL, Scheduler, Ctx, Fs...>&>(new_ctx).next(
//             static_cast<decltype(args)&&>(args)...
//         );
//     }
//
//     template<char... cs, class... Args>
//     static ExecutorResult fallthrough(string_c<cs...>, Args&&... args)
//     {
//         auto& new_ctx = static_cast<Ctx&>(*this);
//         return static_cast<
//             typename JumpSequenceCtx<string_c<cs...>, FL>
//             ::template make<FL, Scheduler, Ctx>&
//         >(new_ctx).next(
//             static_cast<decltype(args)&&>(args)...
//         );
//     }
//
//     template<char... cs>
//     static ExecutorResult next_action(string_c<cs...>)
//     {
//         return Ctx::next_action([](auto ctx, auto&&... args){
//             auto& new_ctx = static_cast<Ctx&>(ctx);
//             return make_lambda<Scheduler>()(
//                 static_cast<
//                     typename JumpSequenceCtx<string_c<cs...>, FL>
//                     ::template make<FL, Scheduler, Ctx>&
//                 >(new_ctx),
//                 static_cast<decltype(args)&&>(args)...
//             );
//         });
//     }
//
//     template<class FF, class... Args>
//     static ExecutorResult try_action(FF, Args&&... args)
//     {
//         auto& new_ctx = static_cast<Ctx&>(*this);
//         Ctx::invoke(
//             make_lambda<FF>(),
//             static_cast<SequenceThenCtx<0, FL, Scheduler, Ctx, FF, Fs...>&>(new_ctx),
//             static_cast<decltype(args)&&>(args)...
//         );
//     }
// };
//
// #ifdef IN_IDE_PARSER
// template<class Scheduler, class... Fs>
// struct SequenceExecutor
// {
//     template<class F>
//     SequenceExecutor then(F) &&;
//
//     template<class CString, class F>
//     SequenceExecutor then(CString, F) &&;
//
//     auto to_function() &&
//     {
//         return [](auto&&...) { return detail::FriendExecutorResult::Nothing; };
//     }
// };
// #else
// template<class Scheduler, class... Fs>
// struct SequenceExecutor
// {
//     template<class F>
//     CXX_WARN_UNUSED_RESULT
//     SequenceExecutor<Scheduler, Fs..., F>
//     then(F) &&
//     {
//         return {};
//     }
//
//     template<char... cs, class F>
//     CXX_WARN_UNUSED_RESULT
//     SequenceExecutor<Scheduler, Fs..., NamedFunction<string_c<cs...>, F>>
//     then(string_c<cs...>, F) &&
//     {
//         return {};
//     }
//
//     CXX_WARN_UNUSED_RESULT
//     auto to_function() &&
//     {
//         return [](auto ctx, auto&&... args){
//             return SequenceExecutor{}(ctx, static_cast<decltype(args)&&>(args)...);
//         };
//     }
//
//     template<class Ctx, class... Args>
//     auto operator()(Ctx ctx, Args&&... args)
//     {
//         return make_lambda<Scheduler>()(
//             static_cast<SequenceCtx<list<Fs...>, Scheduler, decltype(ctx), Fs...>&>(ctx),
//             static_cast<decltype(args)&&>(args)...
//         );
//     }
// };
// #endif
//
//
// template<class Scheduler>
// SequenceExecutor<Scheduler> sequence_executor(Scheduler)
// {
//     return {};
// };
//
// inline auto sequence_executor()
// {
//     return sequence_executor([](auto ctx, auto&&... args){
//         return ctx.next(static_cast<decltype(args)&&>(args)...);
//     });
// };

#include "core/RDP/nla/nla.hpp"

template<auto f>
struct fun_t
{
    template<class Ctx, class T, class... Args>
    ExecutorResult operator()(Ctx&& ctx, T&& o, Args&&... args) const
    {
        if constexpr (std::is_member_function_pointer<decltype(f)>::value) {
            return (o.*f)(ctx, static_cast<Args&&>(args)...);
        }
        else {
            return f(ctx, static_cast<T&&>(o), static_cast<Args&&>(args)...);
        }
    }
};

template<auto mf>
constexpr auto fun = fun_t<mf>{};

struct NewRdpNego
{
public:
    enum {
        EXTENDED_CLIENT_DATA_SUPPORTED = 0x01
    };

    const bool tls;
    const bool nla;

private:
    bool krb;
    bool restricted_admin_mode;

    bool nla_tried = false;

public:
    uint32_t selected_protocol;

private:
    uint32_t enabled_protocols;
    char username[128];

    uint8_t hostname[16];
    uint8_t user[128];
public: // for test_nego
    uint8_t password[2048];
private:
    uint8_t domain[256];
    const char * target_host;

    uint8_t * current_password;
    Random & rand;
    TimeObj & timeobj;
    char * lb_info;

    std::unique_ptr<rdpCredsspClient> credssp;

    std::string& extra_message;
    Translation::language_t lang;

    Error error {NO_ERROR};

    struct RdpNegoProtocols
    {
        enum {
            Rdp = 0x00000001,
            Tls = 0x00000002,
            Nla = 0x00000004
        };
    };

public:
    REDEMPTION_VERBOSE_FLAGS(private, verbose)
    {
        none,
        credssp     = 0x400,
        negotiation = 128,
    };

    struct ServerCert
    {
        const bool              store;
        const ServerCertCheck   check;
        const std::string       path;
        ServerNotifier&     notifier;
    };


    NewRdpNego(
        const bool tls, const char * username, bool nla,
        const char * target_host, const char krb, Random & rand, TimeObj & timeobj,
        std::string& extra_message, Translation::language_t lang,
        const Verbose verbose = {})
    : tls(nla || tls)
    , nla(nla)
    , krb(nla && krb)
    , restricted_admin_mode(false)
    , selected_protocol(RdpNegoProtocols::Rdp)
    , target_host(target_host)
    , current_password(nullptr)
    , rand(rand)
    , timeobj(timeobj)
    , lb_info(nullptr)
    , extra_message(extra_message)
    , lang(lang)
    , verbose(verbose)
    {
        this->enabled_protocols = RdpNegoProtocols::Rdp
            | (this->tls ? RdpNegoProtocols::Tls : 0)
            | (this->nla ? RdpNegoProtocols::Nla : 0);

        LOG(LOG_INFO, "RdpNego: TLS=%s NLA=%s",
            ((this->enabled_protocols & RdpNegoProtocols::Tls) ? "Enabled" : "Disabled"),
            ((this->enabled_protocols & RdpNegoProtocols::Nla) ? "Enabled" : "Disabled")
            );

        strncpy(this->username, username, 127);
        this->username[127] = 0;

        memset(this->hostname, 0, sizeof(this->hostname));
        memset(this->user,     0, sizeof(this->user));
        memset(this->password, 0, sizeof(this->password));
        memset(this->domain,   0, sizeof(this->domain));
    }

    ~NewRdpNego() = default;

    void set_identity(char const * user, char const * domain, char const * pass, char const * hostname)
    {
        if (this->nla) {
            snprintf(reinterpret_cast<char*>(this->user), sizeof(this->user), "%s", user);
            snprintf(reinterpret_cast<char*>(this->domain), sizeof(this->domain), "%s", domain);

            // Password is a multi-sz!
            MultiSZCopy(reinterpret_cast<char*>(this->password), sizeof(this->password), pass);
            this->current_password = this->password;

            snprintf(reinterpret_cast<char*>(this->hostname), sizeof(this->hostname), "%s", hostname);
        }
    }

    void send_negotiation_request(OutTransport trans)
    {
        LOG(LOG_INFO, "RdpNego::send_x224_connection_request_pdu");
        char cookie[256];
        snprintf(cookie, 256, "Cookie: mstshash=%s\x0D\x0A", this->username);
        char * cookie_or_token = this->lb_info ? this->lb_info : cookie;

        if (bool(this->verbose & Verbose::negotiation)) {
            LOG(LOG_INFO, "Send %s:", this->lb_info ? "load_balance_info" : "cookie");
            hexdump_c(cookie_or_token, strlen(cookie_or_token));
        }

        uint32_t rdp_neg_requestedProtocols = X224::PROTOCOL_RDP
            | ((this->enabled_protocols & RdpNegoProtocols::Nla) ?
            X224::PROTOCOL_HYBRID : 0)
            | ((this->enabled_protocols & RdpNegoProtocols::Tls) ?
            X224::PROTOCOL_TLS : 0);

        StaticOutStream<65536> stream;
        X224::CR_TPDU_Send(
            stream, cookie_or_token,
            this->tls ? X224::RDP_NEG_REQ : X224::RDP_NEG_NONE,
            // X224::RESTRICTED_ADMIN_MODE_REQUIRED,
            0, rdp_neg_requestedProtocols);

        trans.send(stream.get_data(), stream.get_offset());
        LOG(LOG_INFO, "RdpNego::send_x224_connection_request_pdu done");
    }

    using ActionCtx = Executor2ActionContext<NewRdpNego&, TpduBuffer&, Transport&, ServerCert>;

    static ExecutorResult exec_recv_data(ActionCtx ctx)
    {
        return ctx.exec_action2(fun<&state_negociate>, fun<&state_recv_connection_confirm>);
    }

private:
    static ExecutorResult state_negociate(
        ActionCtx ctx, NewRdpNego& nego, TpduBuffer& buf, Transport& trans, ServerCert const& cert)
    {
        LOG(LOG_INFO, "RdpNego::NEGO_STATE_%s",
                                (nego.nla) ? "NLA" :
                                (nego.tls) ? "TLS" :
                                              "RDP");
        buf.load_data(trans);
        return nego.state_recv_connection_confirm(ctx, nego, buf, trans, cert);
    }

    static ExecutorResult state_recv_connection_confirm(
        ActionCtx ctx, NewRdpNego& nego, TpduBuffer& buf, OutTransport trans, ServerCert const& /*cert*/)
    {
        while (buf.next_pdu()) {
            LOG(LOG_INFO, "RdpNego::recv_connection_confirm");

            InStream x224_stream(buf.current_pdu_buffer());
            X224::CC_TPDU_Recv x224(x224_stream);

            if (x224.rdp_neg_type == X224::RDP_NEG_NONE){
                nego.enabled_protocols = RdpNegoProtocols::Rdp;
                LOG(LOG_INFO, "RdpNego::recv_connection_confirm done (legacy, no TLS)");
                return ctx.exit_on_success();
            }

            nego.selected_protocol = x224.rdp_neg_code;

            if (x224.rdp_neg_type == X224::RDP_NEG_RSP)
            {
                if (x224.rdp_neg_code == X224::PROTOCOL_HYBRID)
                {
                    LOG(LOG_INFO, "activating SSL");
                    return ctx.exec_action(fun<&state_activate_ssl_hybrid>);
                }

                if (x224.rdp_neg_code == X224::PROTOCOL_TLS)
                {
                    LOG(LOG_INFO, "activating SSL");
                    return ctx.exec_action(fun<&NewRdpNego::state_activate_ssl_tls>);
                }

                if (x224.rdp_neg_code == X224::PROTOCOL_RDP)
                {
                    return ctx.exit_on_success();
                }
            }
            else if (x224.rdp_neg_type == X224::RDP_NEG_FAILURE)
            {
                if (x224.rdp_neg_code == X224::HYBRID_REQUIRED_BY_SERVER)
                {
                    LOG(LOG_INFO, "Enable NLA is probably required");

                    if (!nego.nla_tried) {
                        nego.extra_message = " ";
                        nego.extra_message.append(TR(trkeys::err_nla_required, nego.lang));
                    }
                    trans.disconnect();

                    nego.error = Error(nego.nla_tried
                        ? ERR_NLA_AUTHENTICATION_FAILED
                        : ERR_NEGO_HYBRID_REQUIRED_BY_SERVER);
                    return ctx.exit_on_error();
                }

                if (x224.rdp_neg_code == X224::SSL_REQUIRED_BY_SERVER) {
                    LOG(LOG_INFO, "Enable TLS is probably required");

                    if (!nego.tls) {
                        nego.extra_message = " ";
                        nego.extra_message.append(TR(trkeys::err_tls_required, nego.lang));
                    }
                    trans.disconnect();

                    nego.error = Error(ERR_NEGO_SSL_REQUIRED_BY_SERVER);
                    return ctx.exit_on_error();
                }

                if (x224.rdp_neg_code == X224::SSL_NOT_ALLOWED_BY_SERVER
                || x224.rdp_neg_code == X224::SSL_CERT_NOT_ON_SERVER) {
                    LOG(LOG_INFO, "Can't activate SSL, falling back to RDP legacy encryption");

                    trans.disconnect();
                    if (!trans.connect()){
                        throw Error(ERR_SOCKET_CONNECT_FAILED);
                    }
                    nego.enabled_protocols = RdpNegoProtocols::Rdp;
                    nego.send_negotiation_request(trans);
                }
            }
            else {
                LOG(LOG_INFO, "RdpNego::recv_connection_confirm done");
                return ctx.exit_on_success();
            }
        }

        return ctx.retry();
    }

    bool enable_client_tls(OutTransport trans, ServerCert const& cert)
    {
        switch (trans.enable_client_tls(
            cert.store, cert.check, cert.notifier, cert.path.c_str()))
        {
            case Transport::TlsResult::Want: return false;
            case Transport::TlsResult::Fail:
                LOG(LOG_ERR, "enable_client_tls fail");
                REDEMPTION_CXX_FALLTHROUGH;
            case Transport::TlsResult::Ok: break;
        }
        return true;
    }

    ExecutorResult state_activate_ssl_tls(
        ActionCtx ctx, TpduBuffer& /*buf*/, Transport& trans, ServerCert const& cert)
    {
        if (!this->enable_client_tls(trans, cert)) {
            return ctx.retry();
        }
        return ctx.exit_on_success();
    }

    static ExecutorResult state_activate_ssl_hybrid(
        ActionCtx ctx, NewRdpNego& nego, TpduBuffer& /*buf*/, Transport& trans, ServerCert const& cert)
    {
        // if (x224.rdp_neg_flags & X224::RESTRICTED_ADMIN_MODE_SUPPORTED) {
        //     LOG(LOG_INFO, "Restricted Admin Mode Supported");
        //     nego.restricted_admin_mode = true;
        // }
        if (!nego.enable_client_tls(trans, cert)) {
            return ctx.retry();
        }

        nego.nla_tried = true;

        LOG(LOG_INFO, "activating CREDSSP");
        nego.credssp.reset(new rdpCredsspClient(
            trans, nego.user,
            // nego.domain, nego.password,
            nego.domain, nego.current_password,
            nego.hostname, nego.target_host,
            nego.krb, nego.restricted_admin_mode,
            nego.rand, nego.timeobj, nego.extra_message, nego.lang,
            bool(nego.verbose & Verbose::credssp)
        ));

        if (!nego.credssp->credssp_client_authenticate_init())
        {
            LOG(LOG_INFO, "NLA/CREDSSP Authentication Failed (1)");
            nego.fallback_to_tls(trans);
            return ctx.exec_action2(fun<&state_negociate>, fun<&state_recv_connection_confirm>);
        }

        return ctx.next_action(fun<&state_credssp>);
    }

    void fallback_to_tls(OutTransport trans){
        trans.disconnect();

        if (!trans.connect()){
            LOG(LOG_ERR, "Failed to disconnect transport");
            throw Error(ERR_SOCKET_CONNECT_FAILED);
        }

        this->current_password += (strlen(reinterpret_cast<char*>(this->current_password)) + 1);

        if (*this->current_password) {
            LOG(LOG_INFO, "try next password");
        }
        else {
            LOG(LOG_INFO, "Can't activate NLA");
            LOG(LOG_INFO, "falling back to SSL only");
            this->enabled_protocols = RdpNegoProtocols::Tls | RdpNegoProtocols::Rdp;
        }

        this->send_negotiation_request(trans);
    }

    static ExecutorResult state_credssp(
        ActionCtx ctx, NewRdpNego& nego, TpduBuffer& buf, Transport& trans, ServerCert const& /*cert*/)
    {
        try {
            buf.load_data(trans);
        }
        catch (Error const &) {
            nego.fallback_to_tls(trans);
            return ctx.retry();
        }

        LOG(LOG_INFO, "RdpNego::recv_credssp");

        while (buf.next_credssp()) {
            InStream stream(buf.current_pdu_buffer());
            switch (nego.credssp->credssp_client_authenticate_next(stream))
            {
                case rdpCredsspClient::State::Cont:
                    continue;
                case rdpCredsspClient::State::Err:
                    LOG(LOG_INFO, "NLA/CREDSSP Authentication Failed (2)");
                    nego.fallback_to_tls(trans);
                    if (not *nego.current_password) {
                        return ctx.exec_action2(fun<&state_negociate>, fun<&state_recv_connection_confirm>);
                    }
                    break;
                case rdpCredsspClient::State::Finish:
                    nego.credssp.reset();
                    return ctx.exit_on_success();
            }
        }

        return ctx.retry();
    }
};







RED_AUTO_TEST_CASE(TestNego)
{
    LOG(LOG_INFO, "============= Test Nego Client Side ===========");
    const char client[] =
// RDP Negotiation Request
/* 0000 */ "\x03\x00\x00\x2a\x25\xe0\x00\x00\x00\x00\x00\x43\x6f\x6f\x6b\x69" //...*%......Cooki
/* 0010 */ "\x65\x3a\x20\x6d\x73\x74\x73\x68\x61\x73\x68\x3d\x74\x65\x73\x74" //e: mstshash=test
/* 0020 */ "\x0d\x0a\x01\x00\x08\x00\x03\x00\x00\x00"                         //..........
        // negotiate
/* 0000 */ "\x30\x37\xa0\x03\x02\x01\x02\xa1\x30\x30\x2e\x30\x2c\xa0\x2a\x04" //07......00.0,.*.
/* 0010 */ "\x28\x4e\x54\x4c\x4d\x53\x53\x50\x00\x01\x00\x00\x00\xb7\x82\x08" //(NTLMSSP........
/* 0020 */ "\xe2\x00\x00\x00\x00\x28\x00\x00\x00\x00\x00\x00\x00\x28\x00\x00" //.....(.......(..
/* 0030 */ "\x00\x05\x01\x28\x0a\x00\x00\x00\x0f"                             //...(.....
        // authenticate
/* 0000 */ "\x30\x82\x01\x59\xa0\x03\x02\x01\x02\xa1\x82\x01\x2c\x30\x82\x01" //0..Y........,0..
/* 0010 */ "\x28\x30\x82\x01\x24\xa0\x82\x01\x20\x04\x82\x01\x1c\x4e\x54\x4c" //(0..$... ....NTL
/* 0020 */ "\x4d\x53\x53\x50\x00\x03\x00\x00\x00\x18\x00\x18\x00\x58\x00\x00" //MSSP.........X..
/* 0030 */ "\x00\x70\x00\x70\x00\x70\x00\x00\x00\x0e\x00\x0e\x00\xe0\x00\x00" //.p.p.p..........
/* 0040 */ "\x00\x0c\x00\x0c\x00\xee\x00\x00\x00\x12\x00\x12\x00\xfa\x00\x00" //................
/* 0050 */ "\x00\x10\x00\x10\x00\x0c\x01\x00\x00\xb7\xa2\x88\xe2\x05\x01\x28" //...............(
/* 0060 */ "\x0a\x00\x00\x00\x0f\x50\x89\x6d\x81\x4b\xb6\x53\x8b\x1d\x67\x92" //.....P.m.K.S..g.
/* 0070 */ "\x2e\xd5\x82\x95\x26\x34\xbe\xb5\x6e\xce\xec\x0a\x50\x2d\x29\x63" //....&4..n...P-)c
/* 0080 */ "\x7f\xcc\x5e\xe7\x18\xb8\x6c\xda\xa6\xf0\xf6\x30\x8d\xa6\x41\xcc" //..^...l....0..A.
/* 0090 */ "\x7a\x52\x8e\x7a\xb3\x06\x7d\x0b\xe0\x00\xd5\xf6\x13\x01\x01\x00" //zR.z..}.........
/* 00a0 */ "\x00\x00\x00\x00\x00\x67\x95\x0e\x5a\x4e\x56\x76\xd6\xb8\x6c\xda" //.....g..ZNVv..l.
/* 00b0 */ "\xa6\xf0\xf6\x30\x8d\x00\x00\x00\x00\x01\x00\x08\x00\x57\x00\x49" //...0.........W.I
/* 00c0 */ "\x00\x4e\x00\x37\x00\x02\x00\x08\x00\x57\x00\x49\x00\x4e\x00\x37" //.N.7.....W.I.N.7
/* 00d0 */ "\x00\x03\x00\x08\x00\x77\x00\x69\x00\x6e\x00\x37\x00\x04\x00\x08" //.....w.i.n.7....
/* 00e0 */ "\x00\x77\x00\x69\x00\x6e\x00\x37\x00\x07\x00\x08\x00\x67\x95\x0e" //.w.i.n.7.....g..
/* 00f0 */ "\x5a\x4e\x56\x76\xd6\x00\x00\x00\x00\x00\x00\x00\x00\x49\x00\x74" //ZNVv.........I.t
/* 0100 */ "\x00\x68\x00\x61\x00\x71\x00\x75\x00\x65\x00\x55\x00\x6c\x00\x79" //.h.a.q.u.e.U.l.y
/* 0110 */ "\x00\x73\x00\x73\x00\x65\x00\x54\x00\xe9\x00\x6c\x00\xe9\x00\x6d" //.s.s.e.T...l...m
/* 0120 */ "\x00\x61\x00\x71\x00\x75\x00\x65\x00\x44\xbc\x4d\x7a\x13\x3f\x6b" //.a.q.u.e.D.Mz.?k
/* 0130 */ "\x81\xdb\x1d\x2b\x7b\xbf\x1e\x18\x0f\xa3\x22\x04\x20\x01\x00\x00" //...+{.....". ...
/* 0140 */ "\x00\x89\xe2\xda\x48\x17\x29\xb5\x08\x00\x00\x00\x00\x20\x59\x27" //....H.)...... Y'
/* 0150 */ "\x3f\x08\xd0\xc2\xe4\x75\x66\x10\x49\x7b\xbd\x8d\xf7"             //?....uf.I{...

        // ts credentials (authinfo)
/* 0000 */ "\x30\x5c\xa0\x03\x02\x01\x02\xa2\x55\x04\x53\x01\x00\x00\x00\xaf" //0.......U.S.....
/* 0010 */ "\xad\x46\x2a\x6a\x9d\xf7\x88\x01\x00\x00\x00\xd5\x4f\xc8\xd0\xbd" //.F*j........O...
/* 0020 */ "\x89\x60\xe0\x71\x60\x31\x7a\xcc\xec\xc5\xbf\x23\x4b\xe5\xf9\xa5" //.`.q`1z....#K...
/* 0030 */ "\x8c\x21\x66\xa6\x78\xda\xd1\xbd\xef\xa4\xfd\x47\xa6\xf1\x56\xa5" //.!f.x......G..V.
/* 0040 */ "\xd9\x52\x72\x92\xfa\x41\xa5\xb4\x9d\x94\xfb\x0e\xe2\x61\xba\xfc" //.Rr..A.......a..
/* 0050 */ "\xd5\xf3\xa7\xb5\x33\xd5\x62\x8d\x93\x18\x54\x39\x8a\xe7"         //....3.b...T9..

        ;

    const char server[] =
// RDP Negotiation Response
/* 0000 */ "\x03\x00\x00\x13\x0e\xd0\x00\x00\x00\x00\x00\x02\x00\x08\x00\x02" //................
/* 0010 */ "\x00\x00\x00"                                                     //...

        // challenge
/* 0000 */ "\x30\x81\x88\xa0\x03\x02\x01\x02\xa1\x81\x80\x30\x7e\x30\x7c\xa0" //0..........0~0|.
/* 0010 */ "\x7a\x04\x78\x4e\x54\x4c\x4d\x53\x53\x50\x00\x02\x00\x00\x00\x00" //z.xNTLMSSP......
/* 0020 */ "\x00\x00\x00\x38\x00\x00\x00\xb7\x82\x08\xe2\xb8\x6c\xda\xa6\xf0" //...8........l...
/* 0030 */ "\xf6\x30\x8d\x00\x00\x00\x00\x00\x00\x00\x00\x40\x00\x40\x00\x38" //.0.........@.@.8
/* 0040 */ "\x00\x00\x00\x05\x01\x28\x0a\x00\x00\x00\x0f\x01\x00\x08\x00\x57" //.....(.........W
/* 0050 */ "\x00\x49\x00\x4e\x00\x37\x00\x02\x00\x08\x00\x57\x00\x49\x00\x4e" //.I.N.7.....W.I.N
/* 0060 */ "\x00\x37\x00\x03\x00\x08\x00\x77\x00\x69\x00\x6e\x00\x37\x00\x04" //.7.....w.i.n.7..
/* 0070 */ "\x00\x08\x00\x77\x00\x69\x00\x6e\x00\x37\x00\x07\x00\x08\x00\x67" //...w.i.n.7.....g
/* 0080 */ "\x95\x0e\x5a\x4e\x56\x76\xd6\x00\x00\x00\x00"                     //..ZNVv.....

        // pubauthkey
/* 0000 */ "\x30\x29\xa0\x03\x02\x01\x02\xa3\x22\x04\x20\x01\x00\x00\x00\xa2" //0)......". .....
/* 0010 */ "\xe0\x5b\x50\x97\x8e\x99\x27\x00\x00\x00\x00\xdc\xa7\x0b\xfe\x37" //.[P...'........7
/* 0020 */ "\x45\x3d\x1b\x05\x15\xce\x56\x0a\x54\xa1\xf1"                     //E=....V.T..

        ;
    TestTransport/*struct : TestTransport {
        using TestTransport::TestTransport;

        size_t do_partial_read(uint8_t* buffer, size_t len) override
        {
            return TestTransport::do_partial_read(buffer, 1);
        }
    }*/ logtrans(server, sizeof(server)-1, client, sizeof(client)-1);
    logtrans.set_public_key(reinterpret_cast<const uint8_t*>("1245789652325415"), 16);
    char user[] = "Ulysse";
    char domain[] = "Ithaque";
    char pass[] = "Pénélope\x00";
    char host[] = "Télémaque";
    LCGRandom rand(0);
    LCGTime timeobj;
    NullServerNotifier null_server_notifier;
    std::string extra_message;
    Translation::language_t lang = Translation::EN;
    NewRdpNego nego(true, "test", true, "127.0.0.1", false, rand, timeobj, extra_message, lang);
    using ServerCert = NewRdpNego::ServerCert;
    nego.set_identity(user, domain, pass, host);

    const bool server_cert_store = true;
    TpduBuffer buf;

    nego.send_negotiation_request(logtrans);

    logtrans.disable_remaining_error();

#if defined(__GNUC__) && ! defined(__clang__)
    #define PP_CAT_I(a, b) a##b
    #define PP_CAT(a, b) PP_CAT_I(a, b)
    // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=71332
    #define UNUSED_VARIADIC() [[maybe_unused]] auto&&... \
        PP_CAT(auto_variadic_, PP_CAT(__LINE__, PP_CAT(_, __COUNTER__)))
#else
    #define UNUSED_VARIADIC() auto&&...
#endif

    Reactor reactor;

    {
        using namespace std::chrono_literals;
        TopExecutor2<> executor(reactor);
        executor.set_timeout(10ms);
        auto timer1 = executor.create_timer()
            .on_action(2ms, [](auto ctx){
                TRACE;
                return ctx.retry();
            });
        auto timer2 = executor.create_timer()
            .on_action(3ms, [](auto ctx){
                TRACE;
                return ctx.retry();
            });
        RED_CHECK_EQ(executor.get_next_timeout().count(), 2);
    }



    reactor.create_executor(0, std::ref(nego), std::ref(buf), std::ref(logtrans), ServerCert{
        server_cert_store,
        ServerCertCheck::always_succeed,
        "/tmp/certif",
        null_server_notifier
    })
        .on_action([](auto ctx, UNUSED_VARIADIC()){
            TRACE;
            ctx.create_timer()
                .on_action(std::chrono::milliseconds{}, [](auto ctx){
                    TRACE;
                    return ctx.retry();
                });
            return ctx.exit_on_success();
        })
        .on_exit([](auto ctx, ExecutorError error, UNUSED_VARIADIC()) {
            TRACE;
            RED_CHECK_EQ(error, ExecutorError::NoError);
            return ctx.exit_on_success();
        })
        .on_timeout({}, [](auto ctx, UNUSED_VARIADIC()) {
            TRACE;
            return ctx.exit_on_success();
        })
        .exec_all()
    ;

    std::cout << "-----\n";

    {
        auto* executor = TopExecutor2<>::New(reactor);
        executor->set_on_action([](auto ctx, UNUSED_VARIADIC()){
            TRACE;
            return ctx.exit_on_success();
        });
        executor->set_on_exit([](auto ctx, ExecutorError error, UNUSED_VARIADIC()) {
            TRACE;
            RED_CHECK_EQ(error, ExecutorError::NoError);
            return ctx.exit_on_success();
        });
        executor->exec_all();
        executor->delete_self();
    }

    std::cout << "-----\n";

    auto&& ee = reactor.create_executor(0, std::ref(nego), std::ref(buf), std::ref(logtrans), ServerCert{
        server_cert_store,
        ServerCertCheck::always_succeed,
        "/tmp/certif",
        null_server_notifier
    })
        .on_action([](auto ctx, UNUSED_VARIADIC()){
            TRACE;
            return ctx.exec_sub_executor(0)
                .on_action([](auto ctx, int& i){
                    TRACE;
                    if (++i < 2) {
                        return ctx.retry();
                    }
                    return ctx.exit_on_success();
                    //return nego.exec_recv_data(ctx);
                })
                .on_exit([](auto ctx, ExecutorError error, int) {
                    TRACE;
                    RED_CHECK_EQ(error, ExecutorError::NoError);
                    return ctx.exit_on_success();
                })
            ;
        })
        .on_exit([](auto ctx, ExecutorError error, UNUSED_VARIADIC()) {
            TRACE;
            RED_CHECK_EQ(error, ExecutorError::NoError);
            return ctx.exit_on_success();
        })
        .on_timeout({}, [](auto ctx, UNUSED_VARIADIC()) {
            TRACE;
            return ctx.exit_on_success();
        })
    ;
    ee.exec_all();
    std::cout << "-----\n";
    ee.exec_all();

    std::cout << "-----\n";

    reactor.create_executor(
        0, std::ref(nego), std::ref(buf), std::ref(logtrans), ServerCert{
            server_cert_store,
            ServerCertCheck::always_succeed,
            "/tmp/certif",
            null_server_notifier
        }
    )
        .on_action([](auto ctx, NewRdpNego& nego, UNUSED_VARIADIC()){
            TRACE;
            return nego.exec_recv_data(ctx);
        })
        .on_exit([](auto ctx, ExecutorError error, UNUSED_VARIADIC()) {
            TRACE;
            RED_CHECK_EQ(ExecutorError::NoError, error);
            return ctx.exit_on_success();
        })
        .on_timeout({}, [](auto ctx, UNUSED_VARIADIC()){
            TRACE;
            return ctx.exit_on_success();
        })
    .exec_all();

    buf.consume_current_packet();
    RED_CHECK_EQUAL(0, buf.remaining());
}




// RED_AUTO_TEST_CASE(TestExecutor)
// {
// //     enum class ModState : uint8_t {
// //           MOD_RDP_NEGO
// //         , MOD_RDP_BASIC_SETTINGS_EXCHANGE
// //         , MOD_RDP_CHANNEL_CONNECTION_ATTACH_USER
// //         , MOD_RDP_CHANNEL_JOIN_CONFIRME
// //         , MOD_RDP_GET_LICENSE
// //         , MOD_RDP_CONNECTED
// //     };
// //     TpduBuffer buf;
// //     Transport trans;
// //     LCGRandom rand(0);
// //     LCGTime timeobj;
// //     std::string m;
// //     NewRdpNego nego(false, "", false, "", 'a', rand, timeobj, m, {});
// //     NullServerNotifier null_notifier;
// //     using ServerCert = NewRdpNego::ServerCert;
// //     ServerCert cert {false, {}, {}, null_notifier};
//
//     Executor executor;
//
// //     executor.create_executor(std::ref(nego), std::ref(buf), std::ref(trans), std::ref(cert))
// //         .on_action([](auto ctx, NewRdpNego& nego, TpduBuffer& buf, Transport& trans, ServerCert const& cert){
// //             TRACE;
// //             if (nego.recv_data(buf, trans, cert)) {
// //                 return ctx.retry();
// //             }
// //             if (nego.get_state() == NewRdpNego::State::Final){
// //                 return ctx.exit_on_success();
// //             }
// //             return ctx.exit_on_error();
// //         })
// //         .on_timeout([](auto ctx, NewRdpNego&, TpduBuffer&, Transport&, ServerCert const&) {
// //             TRACE;
// //             return ctx.exit_on_success();
// //         })
// //         .on_exit([](auto ctx, bool, NewRdpNego&, TpduBuffer&, Transport&, ServerCert const&) {
// //             TRACE;
// //             return ctx.exit_on_success();
// //         })
// //     ;
// //
// //     executor.exec_all();
//
//
//     executor.create_executor(1, 2)
//         .on_action([](auto ctx, int, int){
//             TRACE;
//             return ctx.sub_executor(0)
//                 .on_action([](auto ctx, int& i){
//                     TRACE;
//                     if (++i < 5) {
//                         return ctx.retry();
//                     }
//                     return ctx.exit_on_success();
//                 })
//                 .on_timeout([](auto ctx, int){
//                     TRACE;
//                     return ctx.exit_on_success();
//                 })
//                 .on_exit([](auto ctx, bool, int){
//                     TRACE;
//                     return ctx.exit_on_success();
//                 })
//             ;
//         })
//         .on_exit([](auto ctx, bool, int, int) {
//             TRACE;
//             return ctx.exit_on_success();
//         })
//         .on_timeout([](auto ctx, int, int) {
//             TRACE;
//             return ctx.exit_on_success();
//         })
//     ;
//
//     executor.exec_all();
//
//
//     executor.create_executor(0)
//         .on_action(
//             sequence_executor([](auto ctx, int& n){
//                 ++n;
//                 TRACE_n(n);
//                 if (n & 1) {
//                     return ctx.next(n);
//                 }
//                 return ctx.retry();
//             })
//             .then("10"_cs, [](auto ctx, int& i){
//                 TRACE;
//                 return ctx.fallthrough(i);
//             })
//             .then([](auto ctx, int& i){
//                 TRACE;
//                 return ++i < 5 ? ctx.retry() : ctx.exit_on_success();
//             })
//             .then([](auto ctx, int& i){
//                 TRACE;
//                 return ++i < 10 ? ctx.retry() : ctx.fallthrough(i);
//             })
//             .then([](auto ctx, int& i){
//                 TRACE;
//                 return i > 30 ? ctx.exit_on_success()
//                    : ++i < 15 ? ctx.retry() : ctx.fallthrough("10"_cs, i);
//             })
//         )
//         .on_exit([](auto ctx, bool, int) {
//             TRACE;
//             return ctx.exit_on_success();
//         })
//         .on_timeout([](auto ctx, int) {
//             TRACE;
//             return ctx.exit_on_success();
//         })
//     ;
//
//     executor.exec_all();
// }
