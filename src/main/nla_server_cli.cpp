/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2019
   Author(s): Christophe Grosjean

   An isolated front-end RDP Server with NLA support
*/

#include "proxy_recorder/nego_server.hpp"

#include "core/RDP/x224.hpp"
#include "core/RDP/nla/nla_server_ntlm.hpp"
#include "core/RDP/nla/nla_client_ntlm.hpp"
#include "core/RDP/nla/nla_client_kerberos.hpp"
#include "core/RDP/gcc/userdata/cs_core.hpp"
#include "core/RDP/gcc.hpp"
#include "core/RDP/mcs.hpp"
#include "core/RDP/tpdu_buffer.hpp"
#include "core/listen.hpp"
#include "core/server_notifier_api.hpp"
#include "proxy_recorder/extract_user_domain.hpp"
#include "transport/recorder_transport.hpp"
#include "transport/socket_transport.hpp"
#include "transport/socket_trace_transport.hpp"
#include "utils/cli.hpp"
#include "utils/fixed_random.hpp"
#include "utils/netutils.hpp"
#include "utils/redemption_info_version.hpp"
#include "utils/utf.hpp"
#include "system/scoped_ssl_init.hpp"

#include <vector>
#include <chrono>
#include <iostream>

#include <cerrno>
#include <cstring>
#include <csignal>

#include <netinet/tcp.h>
#include <sys/select.h>
#include <openssl/ssl.h>




/** @brief the server that handles RDP connections */
class NLAServer
{
    std::unique_ptr<NegoServer> nego_server;
    std::string nla_username;
    std::string nla_password;

    std::pair<PasswordCallback,array_md4> get_password_hash(bytes_view user_av, bytes_view domain_av)
    {
        LOG(LOG_INFO, "NTLM Check identity");
        hexdump_d(user_av);

        auto bytes_to_zstring_view = [](std::vector<uint8_t>& cont){
            cont.push_back(0);
            return zstring_view(zstring_view::is_zero_terminated{},
                byte_ptr(cont.data()).as_charp(), cont.size());
        };

        auto [username, domain] = extract_user_domain(this->nla_username);
        // from protocol
        auto tmp_utf8_user = ::encode_UTF16_to_UTF8(user_av);
        auto u8user = bytes_to_zstring_view(tmp_utf8_user);
        auto tmp_utf8_domain = ::encode_UTF16_to_UTF8(domain_av);
        auto u8domain = bytes_to_zstring_view(tmp_utf8_domain);

        LOG(LOG_INFO, "NTML IDENTITY(message): identity.User=%s identity.Domain=%s username=%s, domain=%s",
            u8user, u8domain, username, domain);

        if (u8domain.size() == 0){
            auto [identity_username, identity_domain] = extract_user_domain(u8user.to_sv());

            bool user_match = username == identity_username;
            bool domain_match = domain == identity_domain;

            if (user_match && domain_match){
                LOG(LOG_INFO, "known identity");
                return {PasswordCallback::Ok, Md4(::UTF8toUTF16(this->nla_password))};
            }
        }
        else if (u8user == username && u8domain == domain){
            return {PasswordCallback::Ok, Md4(::UTF8toUTF16(this->nla_password))};
        }

        LOG(LOG_ERR, "Ntlm: unknwon identity");
        return {PasswordCallback::Error, {}};
    }

    enum class PState : unsigned {
        NEGOTIATING_FRONT_HELLO,
        NEGOTIATING_FRONT_NLA,
        NEGOTIATING_FRONT_INITIAL_PDU,
        NEGOTIATING_FINISH,
    } pstate = PState::NEGOTIATING_FRONT_HELLO;

    X224::CR_TPDU_Data front_CR_TPDU;

    uint8_t front_public_key[1024] = {};
    writable_u8_array_view front_public_key_av;

public:
    NLAServer(std::string nla_username, std::string nla_password, bool forkable, const TimeBase & time_base, uint64_t verbosity)
        : nla_username(std::move(nla_username))
        , nla_password(std::move(nla_password))
        , forkable(forkable)
        , time_base(time_base)
        , verbosity(verbosity)
    {
        // just ignore this signal because there is no child termination management yet.
        struct sigaction sa;
        sa.sa_flags = 0;
        sigaddset(&sa.sa_mask, SIGCHLD);
        sa.sa_handler = SIG_IGN; /*NOLINT*/
        sigaction(SIGCHLD, &sa, nullptr);
    }

    X224::CR_TPDU_Data front_hello(Transport & trans, bytes_view tpdu, bool bogus_neg_req, int verbosity)
    {
        X224::CR_TPDU_Data cr_tpdu;

        LOG(LOG_INFO, "front RDP Hello");

        InStream x224_stream(tpdu);
        cr_tpdu = X224::CR_TPDU_Data_Recv(x224_stream, bogus_neg_req, verbosity);
        if (cr_tpdu._header_size != x224_stream.get_capacity()) {
            LOG(LOG_WARNING,
                "Front::incoming: connection request: all data should have been consumed,"
                " %zu bytes remains",
                x224_stream.get_capacity() - cr_tpdu._header_size);
        }

        LOG_IF((verbosity > 8) && (cr_tpdu.rdp_neg_requestedProtocols & X224::PROTOCOL_TLS), LOG_INFO, "TLS Front");
        LOG_IF((verbosity > 8) && (cr_tpdu.rdp_neg_requestedProtocols & X224::PROTOCOL_HYBRID), LOG_INFO, "Hybrid (NLA) Front");

        StaticOutStream<256> front_x224_stream;
        X224::CC_TPDU_Send(
            front_x224_stream,
            X224::RDP_NEG_RSP,
            RdpNego::EXTENDED_CLIENT_DATA_SUPPORTED,
            (cr_tpdu.rdp_neg_requestedProtocols & X224::PROTOCOL_HYBRID)?X224::PROTOCOL_HYBRID:
            (cr_tpdu.rdp_neg_requestedProtocols & X224::PROTOCOL_TLS)?X224::PROTOCOL_TLS:
            X224::PROTOCOL_RDP);
        trans.send(front_x224_stream.get_produced_bytes());

        if ((cr_tpdu.rdp_neg_requestedProtocols & X224::PROTOCOL_TLS)
        || (cr_tpdu.rdp_neg_requestedProtocols & X224::PROTOCOL_HYBRID)) {
            trans.enable_server_tls("inquisition", nullptr, 0 /* tls_min_level */, 0  /* tls_max_level */, true);
            bytes_view key = trans.get_public_key();
            memcpy(this->front_public_key, key.data(), key.size());
            this->front_public_key_av = writable_array_view{this->front_public_key, key.size()};
        }

        if (cr_tpdu.rdp_neg_requestedProtocols & X224::PROTOCOL_HYBRID) {
            if (this->verbosity > 4) {
                LOG(LOG_INFO, "start NegoServer");
            }
            this->nego_server = std::make_unique<NegoServer>(this->front_public_key_av, time_base, true);
            this->pstate = PState::NEGOTIATING_FRONT_NLA;
        }
        return cr_tpdu;
    }

    void front_nla(Transport & trans, bytes_view buffer)
    {
        LOG(LOG_INFO, "starting NLA NegoServer");
        std::vector<uint8_t> result;
        credssp::State st = credssp::State::Cont;
        LOG(LOG_INFO, "NegoServer recv_data authenticate_next");
        if (this->nego_server->credssp.ntlm_state == NTLM_STATE_WAIT_PASSWORD){
            result << this->nego_server->credssp.authenticate_next({});
        }
        else {
            result << this->nego_server->credssp.authenticate_next(buffer);
        }
        st = this->nego_server->credssp.state;
        if (result.size() > 0){ // If waiting for password, no data
            trans.send(result);
        }

        switch (st) {
        case credssp::State::Err: {
            LOG(LOG_INFO, "NLA NegoServer Authentication Failed");
            throw Error(ERR_NLA_AUTHENTICATION_FAILED);
        }
        case credssp::State::Cont: {
            LOG(LOG_INFO, "NLA NegoServer Running");
        }
        break;
        case credssp::State::Finish:
            LOG(LOG_INFO, "NLA NegoServer Done");
            this->pstate = PState::NEGOTIATING_FRONT_INITIAL_PDU;
            break;
        }
    }

    void front_initial_pdu_negociation(TpduBuffer & buffer)
    {
        LOG(LOG_INFO, "RDP Init");
        writable_u8_array_view currentPacket = buffer.current_pdu_buffer();

        if (!this->nla_username.empty()) {
            if (this->verbosity > 4) {
                LOG(LOG_INFO, "Back: force protocol PROTOCOL_HYBRID");
            }
            InStream new_x224_stream(currentPacket);
            X224::DT_TPDU_Recv x224(new_x224_stream);
            MCS::CONNECT_INITIAL_PDU_Recv mcs_ci(x224.payload, MCS::BER_ENCODING);
            GCC::Create_Request_Recv gcc_cr(mcs_ci.payload);
            GCC::UserData::RecvFactory f(gcc_cr.payload);
            // force X224::PROTOCOL_HYBRID
            if (f.tag == CS_CORE) {
                GCC::UserData::CSCore cs_core;
                cs_core.recv(f.payload);
                if (cs_core.length >= 216) {
                    auto const idx = f.payload.get_current() - currentPacket.data() + (216-cs_core.length) - 4;
                    currentPacket[idx] = X224::PROTOCOL_HYBRID;
                }
            }
            this->pstate = PState::NEGOTIATING_FINISH;
        }
    }

    bool start(int sck)
    {
        LOG(LOG_INFO, "Starting FrontServer");
        unique_fd sck_in {accept(sck, nullptr, nullptr)};
        if (!sck_in) {
            LOG(LOG_ERR, "Accept failed on socket %d (%s)", sck, strerror(errno));
            _exit(1);
        }

        const pid_t pid = this->forkable ? fork() : 0;
        connection_counter++;

        if(pid == 0) {
            close(sck);

            // int nodelay = 1;
            // if (setsockopt(sck_in.fd(), IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay)) < 0) {
            //     LOG(LOG_ERR, "Failed to set socket TCP_NODELAY option on client socket");
            //     _exit(1);
            // }

            const auto sck_verbose = safe_cast<SocketTransport::Verbose>(uint32_t(verbosity >> 32));

            TimeBase time_base(tvtime());

            TpduBuffer buffer;
            buffer.trace_pdu = true;
            SocketTransport trans(
                "front", std::move(sck_in), "127.0.0.1", 3389,
                std::chrono::milliseconds(100), sck_verbose);

            try {
                fd_set rset;
                int const front_fd = trans.get_fd();

                for (;;) {
                    FD_ZERO(&rset);
                    FD_SET(front_fd, &rset);
                    int status = select(front_fd + 1, &rset, nullptr, nullptr, nullptr);
                    time_base.set_current_time(tvtime());
                    if (status < 0) {
                        std::cerr << "Selected returned an error\n";
                        break;
                    }

                    if (FD_ISSET(front_fd, &rset)) {
                        LOG(LOG_INFO, "Data Received from Client");
                        buffer.load_data(trans);
                        switch(pstate) {
                        case PState::NEGOTIATING_FRONT_HELLO:
                            LOG(LOG_INFO, "NEGOTIATING_FRONT_HELLO");
                            if (buffer.next(TpduBuffer::PDU)) {
                                bool bogus_neg_req = false;
                                this->front_CR_TPDU = this->front_hello(trans, buffer.current_pdu_buffer(), bogus_neg_req, this->verbosity);
                            }
                            break;

                        case PState::NEGOTIATING_FRONT_NLA:
                            LOG(LOG_INFO, "NEGOTIATING_FRONT_NLA");
                            if (buffer.next(TpduBuffer::CREDSSP)) {
                                this->front_nla(trans, buffer.current_pdu_buffer());
                            }
                            if (this->nego_server->credssp.ntlm_state == NTLM_STATE_WAIT_PASSWORD){
                                auto [password_res, password_hash] = get_password_hash(
                                    this->nego_server->credssp.authenticate.UserName.buffer,
                                    this->nego_server->credssp.authenticate.DomainName.buffer);

                                this->nego_server->credssp.set_password_hash(password_res, password_hash);
                                this->front_nla(trans, {});
                            }
                            break;

                        case PState::NEGOTIATING_FRONT_INITIAL_PDU:
                            LOG(LOG_INFO, "NEGOTIATING_FRONT_INITIAL_PDU");
                            if (buffer.next(TpduBuffer::PDU)) {
                                this->front_initial_pdu_negociation(buffer);
                            }
                            break;
                        case PState::NEGOTIATING_FINISH:
                            LOG(LOG_INFO, "Negotiating finished");
                            break;
                        }
                    }
                }
            } catch(Error const& e) {
                if (errno) {
                    LOG(LOG_ERR, "Recording front connection ending: %s ; %s", e.errmsg(), strerror(errno));
                }
                else {
                    LOG(LOG_ERR, "Recording front connection ending: %s", e.errmsg());
                }
                LOG(LOG_INFO, "Exiting FrontServer (1)");
                exit(1);
            }
            LOG(LOG_INFO, "Exiting FrontServer (0)");
            exit(0);
        }
        else if (!this->forkable) {
            return false;
        }

        return true;
    }

private:
    int connection_counter = 0;
//    bool enable_kerberos;
    bool forkable;
    const TimeBase & time_base;
    uint64_t verbosity;
};

int main(int argc, char *argv[])
{
    int listen_port = 8001;
    std::string nla_username;
    std::string nla_password;
    TimeBase time_base(tvtime());
    bool no_forkable = false;
    bool enable_kerberos = false;
    uint64_t verbosity = 0;

    auto options = cli::options(
        cli::option('h', "help").help("Show help").parser(cli::help()),
        cli::option('v', "version").help("Show version")
            .parser(cli::quit([]{ std::cout << "NLAFront 1.0, " << redemption_info_version() << "\n"; })),
        cli::option('P', "port").help("Listen port").parser(cli::arg_location(listen_port)),
        cli::option("user").parser(cli::arg_location(nla_username)).argname("<username>"),
        cli::option("pass").parser(cli::password_location(argv, nla_password)),
        cli::option("enable-kerberos").parser(cli::on_off_location(enable_kerberos)),
        cli::option('N', "no-fork").parser(cli::on_off_location(no_forkable)),
        cli::option('V', "verbose").parser(cli::arg_location(verbosity)).argname("<verbosity>")
    );

    auto cli_result = cli::parse(options, argc, argv);
    switch (cli_result.res) {
        case cli::Res::Ok:
            break;
        case cli::Res::Exit:
            return 0;
        case cli::Res::Help:
            cli::print_help(options, std::cout);
            return 0;
        case cli::Res::BadFormat:
        case cli::Res::BadOption:
        case cli::Res::NotOption:
        case cli::Res::StopParsing:
            std::cerr << "Bad " << (cli_result.res == cli::Res::BadFormat ? "format" : "option") << " at parameter " << cli_result.opti;
            if (cli_result.opti < cli_result.argc) {
                std::cerr << " (" << cli_result.argv[cli_result.opti] << ")";
            }
            std::cerr << "\n";
            return 1;
    }

    ScopedSslInit scoped_ssl;

    openlog("NLAServer", LOG_CONS | LOG_PERROR, LOG_USER);

    NLAServer front(std::move(nla_username), std::move(nla_password), !no_forkable, time_base, verbosity);
    auto sck = create_server(inet_addr("0.0.0.0"), listen_port, EnableTransparentMode::No);
    if (!sck) {
        return 2;
    }
    return unique_server_loop(std::move(sck), [&](int sck){
        time_base.set_current_time(tvtime());
        return front.start(sck);
    });
}
