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
   Copyright (C) Wallix 2018
   Author(s): David Fort

   A proxy that will capture all the traffic to the target
*/

#include "core/RDP/nla/nla.hpp"
#include "core/RDP/gcc.hpp"
#include "core/RDP/mcs.hpp"
#include "core/RDP/nego.hpp"
#include "core/RDP/tpdu_buffer.hpp"
#include "core/listen.hpp"
#include "core/server_notifier_api.hpp"
#include "transport/recorder_transport.hpp"
#include "transport/socket_transport.hpp"
#include "utils/cli.hpp"
#include "utils/fixed_random.hpp"
#include "utils/netutils.hpp"
#include "utils/redemption_info_version.hpp"
#include "utils/utf.hpp"

#include <vector>
#include <chrono>
#include <iostream>

#include <cerrno>
#include <cstring>
#include <csignal>

#include <netinet/tcp.h>
#include <sys/select.h>
#include <openssl/ssl.h>


using PacketType = RecorderFile::PacketType;


/// @brief Wrap a Transport and a RecorderFile for a nla negociation
struct NlaTeeTransport : Transport
{
    enum class Type : bool { Client, Server };

    NlaTeeTransport(Transport& trans, RecorderFile& outFile, Type type)
    : trans(trans)
    , outFile(outFile)
    , is_server(type == Type::Server)
    {}

    TlsResult enable_client_tls(
        bool server_cert_store,
        ServerCertCheck server_cert_check,
        ServerNotifier & server_notifier,
        const char * certif_path
    ) override
    {
        return this->trans.enable_client_tls(
            server_cert_store, server_cert_check, server_notifier, certif_path);
    }

    array_view_const_u8 get_public_key() const override
    {
        return this->trans.get_public_key();
    }

    bool disconnect() override
    {
        return this->trans.disconnect();
    }

    bool connect() override
    {
        return this->trans.connect();
    }

private:
    Read do_atomic_read(uint8_t * buffer, size_t len) override
    {
        auto r = this->trans.atomic_read(buffer, len);
        outFile.write_packet(
            this->is_server ? PacketType::NlaServerIn : PacketType::NlaClientIn,
            {buffer, len});
        return r;
    }

    size_t do_partial_read(uint8_t * buffer, size_t len) override
    {
        len = this->trans.partial_read(buffer, len);
        outFile.write_packet(
            this->is_server ? PacketType::NlaServerIn : PacketType::NlaClientIn,
            {buffer, len});
        return len;
    }

    void do_send(const uint8_t * buffer, size_t len) override
    {
        outFile.write_packet(
            this->is_server ? PacketType::NlaServerOut : PacketType::NlaClientOut,
            {buffer, len});
        this->trans.send(buffer, len);
    }

private:
    Transport& trans;
    RecorderFile& outFile;
    bool is_server;
};


/** @brief a front connection with a RDP client */
class FrontConnection
{
public:
    FrontConnection(unique_fd sck, std::string host, int port, std::string const& captureFile,
        TimeObj& timeobj,std::string nla_username, std::string nla_password, bool enable_kerberos,
        uint64_t verbosity
    )
        : frontConn("front", std::move(sck), "127.0.0.1", 3389, std::chrono::milliseconds(100), to_verbose_flags(verbosity))
        , backConn("back", ip_connect(host.c_str(), port, 3, 1000),
            host.c_str(), port, std::chrono::milliseconds(100), to_verbose_flags(verbosity))
        , outFile(timeobj, captureFile.c_str())
        , host(std::move(host))
        , nla_username(std::move(nla_username))
        , nla_password(std::move(nla_password))
        , enable_kerberos(enable_kerberos)
        , verbosity(verbosity)
    {
        if (!this->nla_password.empty()) {
            this->nla_password.push_back('\0');
        }
    }

    void treat_front_activity()
    {
        switch (state) {
        case NEGOCIATING_FRONT_STEP1:
            frontBuffer.load_data(this->frontConn);
            if (frontBuffer.next_pdu()) {
                array_view_u8 currentPacket = frontBuffer.current_pdu_buffer();
                InStream x224_stream(currentPacket);
                X224::CR_TPDU_Recv x224(x224_stream, true);
                if (x224._header_size != x224_stream.get_capacity()) {
                    LOG(LOG_WARNING,
                        "Front::incoming: connection request: all data should have been consumed,"
                        " %zu bytes remains",
                        x224_stream.get_capacity() - x224._header_size);
                }

                is_tls_client = (x224.rdp_neg_requestedProtocols & X224::PROTOCOL_TLS);
                is_nla_client = (x224.rdp_neg_requestedProtocols & X224::PROTOCOL_HYBRID);

                StaticOutStream<256> front_x224_stream;
                X224::CC_TPDU_Send(
                    front_x224_stream,
                    X224::RDP_NEG_RSP,
                    RdpNego::EXTENDED_CLIENT_DATA_SUPPORTED,
                    select_client_protocol());
                outFile.write_packet(PacketType::DataIn, front_x224_stream.get_bytes());
                frontConn.send(front_x224_stream.get_bytes());

                if (is_tls_client || is_nla_client) {
                    frontConn.enable_server_tls("inquisition", nullptr);
                }

                if (is_nla_client) {
                    LOG(LOG_INFO, "start NegoServer");
                    nego_server = std::make_unique<NegoServer>(
                        frontConn, outFile, nla_username, nla_password, verbosity);

                    rdpCredsspServer::State st = nego_server->recv_data(frontBuffer);

                    if (rdpCredsspServer::State::Err == st) {
                        throw Error(ERR_NLA_AUTHENTICATION_FAILED);
                    }
                }

                nego_client = std::make_unique<NegoClient>(
                    !nla_username.empty(),
                    x224.rdp_cinfo_flags & X224::RESTRICTED_ADMIN_MODE_REQUIRED,
                    backConn, outFile,
                    host.c_str(), nla_username.c_str(),
                    nla_password.empty() ? "\0" : nla_password.c_str(),
                    enable_kerberos, verbosity);

                // equivalent to nego_client->send_negotiation_request()
                StaticOutStream<256> back_x224_stream;
                X224::CR_TPDU_Send(
                    back_x224_stream, x224.cookie, x224.rdp_neg_type, x224.rdp_neg_flags,
                    !nla_username.empty() ? X224::PROTOCOL_HYBRID : X224::PROTOCOL_TLS);
                outFile.write_packet(PacketType::DataOut, back_x224_stream.get_bytes());
                backConn.send(back_x224_stream.get_bytes());

                state = nego_server ? NEGOCIATING_FRONT_NLA : NEGOCIATING_BACK_NLA;
            }
            break;

        case NEGOCIATING_FRONT_NLA: {
            frontBuffer.load_data(frontConn);
            rdpCredsspServer::State st = nego_server->recv_data(frontBuffer);
            switch (st) {
            case rdpCredsspServer::State::Err: throw Error(ERR_NLA_AUTHENTICATION_FAILED);
            case rdpCredsspServer::State::Cont: break;
            case rdpCredsspServer::State::Finish:
                LOG(LOG_INFO, "stop NegoServer");
                LOG(LOG_INFO, "start NegoClient");
                nego_server.reset();
                state = NEGOCIATING_BACK_NLA;
                break;
            }
            break;
        }

        // force X224::PROTOCOL_HYBRID
        case NEGOCIATING_FRONT_INITIAL_PDU:
            frontBuffer.load_data(this->frontConn);
            if (frontBuffer.next_pdu()) {
                array_view_u8 currentPacket = frontBuffer.current_pdu_buffer();

                if (!nla_username.empty()) {
                    LOG(LOG_INFO, "Back: force protocol PROTOCOL_HYBRID");
                    InStream new_x224_stream(currentPacket);
                    X224::DT_TPDU_Recv x224(new_x224_stream);
                    MCS::CONNECT_INITIAL_PDU_Recv mcs_ci(x224.payload, MCS::BER_ENCODING);
                    GCC::Create_Request_Recv gcc_cr(mcs_ci.payload);
                    GCC::UserData::RecvFactory f(gcc_cr.payload);
                    if (f.tag == CS_CORE) {
                        GCC::UserData::CSCore cs_core;
                        cs_core.recv(f.payload);
                        if (cs_core.length > 216) {
                            auto const idx = f.payload.get_current() - currentPacket.data() - 4;
                            currentPacket[idx] = X224::PROTOCOL_HYBRID;
                        }
                    }
                }

                outFile.write_packet(PacketType::DataOut, frontBuffer.remaining_data());
                backConn.send(frontBuffer.remaining_data());

                state = NEGOCIATING_BACK_INITIAL_PDU;
            }
            break;
        case FORWARD: {
            size_t ret = frontConn.partial_read(make_array_view(tmpBuffer));
            if (ret > 0) {
                outFile.write_packet(PacketType::DataOut, {tmpBuffer, ret});
                backConn.send(tmpBuffer, ret);
            }
            break;
        }
        case NEGOCIATING_BACK_NLA:
        case NEGOCIATING_BACK_INITIAL_PDU:
            REDEMPTION_UNREACHABLE();
        }
    }

    void treat_back_activity()
    {
        switch (state) {
        case NEGOCIATING_BACK_NLA: {
            NullServerNotifier null_notifier;
            if (not nego_client->recv_next_data(backBuffer, null_notifier)) {
                LOG(LOG_INFO, "stop NegoClient");
                this->nego_client.reset();
                state = NEGOCIATING_FRONT_INITIAL_PDU;
                outFile.write_packet(PacketType::ClientCert, backConn.get_public_key());
            }
            break;
        }
        case NEGOCIATING_BACK_INITIAL_PDU: {
            backBuffer.load_data(this->backConn);
            if (backBuffer.next_pdu()) {
                array_view_u8 currentPacket = backBuffer.current_pdu_buffer();

                if (!nla_username.empty()) {
                    LOG(LOG_INFO, "Front: force protocol tls=%d nla=%d", is_tls_client, is_nla_client);
                    InStream new_x224_stream(currentPacket);
                    X224::DT_TPDU_Recv x224(new_x224_stream);
                    MCS::CONNECT_RESPONSE_PDU_Recv mcs(x224.payload, MCS::BER_ENCODING);
                    GCC::Create_Response_Recv gcc_cr(mcs.payload);
                    GCC::UserData::RecvFactory f(gcc_cr.payload);
                    if (f.tag == SC_CORE) {
                        GCC::UserData::SCCore sc_core;
                        sc_core.recv(f.payload);
                        if (sc_core.length >= 12) {
                            hexdump(f.payload.get_data(), f.payload.get_capacity());
                            auto const offset = (sc_core.length >= 16) ? 8 : 4;
                            auto const idx = f.payload.get_current() - currentPacket.data() - offset;
                            currentPacket[idx] = select_client_protocol();
                            hexdump(f.payload.get_data(), f.payload.get_capacity());
                        }
                    }
                }

                outFile.write_packet(PacketType::DataIn, backBuffer.remaining_data());
                frontConn.send(backBuffer.remaining_data());

                state = FORWARD;
            }
            break;
        }
        case FORWARD: {
            size_t ret = backConn.partial_read(make_array_view(tmpBuffer));
            if (ret > 0) {
                frontConn.send(tmpBuffer, ret);
                outFile.write_packet(PacketType::DataIn, {tmpBuffer, ret});
            }
            break;
        }
        case NEGOCIATING_FRONT_INITIAL_PDU:
        case NEGOCIATING_FRONT_STEP1:
        case NEGOCIATING_FRONT_NLA:
            REDEMPTION_UNREACHABLE();
        }
    }

    void run()
    {
        fd_set rset;
        int const front_fd = frontConn.get_fd();
        int const back_fd = backConn.get_fd();

        frontConn.enable_trace = bool(verbosity & 1);
        backConn.enable_trace = bool(verbosity & 1);

        for (;;) {
            FD_ZERO(&rset);

            switch(state) {
            case NEGOCIATING_FRONT_NLA:
            case NEGOCIATING_FRONT_STEP1:
            case NEGOCIATING_FRONT_INITIAL_PDU:
                FD_SET(front_fd, &rset);
                break;
            case NEGOCIATING_BACK_NLA:
            case NEGOCIATING_BACK_INITIAL_PDU:
                FD_SET(back_fd, &rset);
                break;
            case FORWARD:
                FD_SET(front_fd, &rset);
                FD_SET(back_fd, &rset);
                break;
            }

            int status = select(std::max(front_fd, back_fd) + 1, &rset, nullptr, nullptr, nullptr);
            if (status < 0) {
                break;
            }

            if (FD_ISSET(front_fd, &rset)) {
                treat_front_activity();
            }

            if (FD_ISSET(back_fd, &rset)) {
                treat_back_activity();
            }
        }
    }

private:
    uint8_t select_client_protocol() const
    {
        return is_nla_client ? X224::PROTOCOL_HYBRID :
               is_tls_client ? X224::PROTOCOL_TLS
                             : X224::PROTOCOL_RDP;
    }

    static std::pair<std::string, std::string>
    extract_user_domain(char const* target_user)
    {
        std::pair<std::string, std::string> ret;
        auto& [username, domain] = ret;

        char const* separator = strchr(target_user, '\\');
        if (separator) {
            username = separator+1;
            domain.assign(target_user, separator-target_user);
        }
        else {
            separator = strchr(target_user, '@');
            if (separator) {
                username.assign(target_user, separator-target_user);
                domain = separator+1;
            }
            else {
                username = target_user;
            }
        }

        return ret;
    }

    class NegoServer
    {
        FixedRandom rand;
        LCGTime timeobj;
        std::string extra_message;
        NlaTeeTransport trans;
        rdpCredsspServer credssp;

    public:
        NegoServer(
            Transport& trans, RecorderFile& outFile,
            std::string const& user, std::string const& password,
            uint64_t verbosity)
        : trans(trans, outFile, NlaTeeTransport::Type::Server)
        , credssp(
            this->trans, false, false, rand, timeobj, extra_message, Translation::EN,
            [&](SEC_WINNT_AUTH_IDENTITY& identity){
                auto check = [vec = std::vector<uint8_t>{}](
                    std::string const& str, Array& arr
                ) mutable {
                    vec.resize(str.size() * 2);
                    UTF8toUTF16(str, vec.data(), vec.size());
                    return vec.size() == arr.size()
                        && 0 == memcmp(vec.data(), arr.get_data(), vec.size());
                };

                auto [username, domain] = extract_user_domain(user.c_str());
                if (check(username, identity.User)
                 // domain is empty
                 && check(domain, identity.Domain)
                ) {
                    identity.SetPasswordFromUtf8(byte_ptr_cast(password.c_str()));
                    return Ntlm_SecurityFunctionTable::PasswordCallback::Ok;
                }

                LOG(LOG_ERR, "Ntlm: bad identity");
                return Ntlm_SecurityFunctionTable::PasswordCallback::Error;
            }, verbosity)
        {
            this->credssp.credssp_server_authenticate_init();
        }

        rdpCredsspServer::State recv_data(TpduBuffer& buffer)
        {
            rdpCredsspServer::State st = rdpCredsspServer::State::Cont;
            while (buffer.next_credssp() && rdpCredsspServer::State::Cont == st) {
                InStream in_stream(buffer.current_pdu_buffer());
                st = this->credssp.credssp_server_authenticate_next(in_stream);
            }
            return st;
        }
    };

    class NegoClient
    {
        LCGTime timeobj;
        FixedRandom random;
        std::string extra_message;
        RdpNego nego;
        NlaTeeTransport trans;

    public:
        NegoClient(
            bool is_nla, bool is_admin_mode,
            Transport& trans, RecorderFile& outFile,
            char const* host, char const* target_user, char const* password,
            bool enable_kerberos, uint64_t verbosity
        )
        : nego(true, target_user, is_nla, is_admin_mode, host, enable_kerberos,
            this->random, this->timeobj, this->extra_message, Translation::EN,
            to_verbose_flags(verbosity))
        , trans(trans, outFile, NlaTeeTransport::Type::Client)
        {
            auto [username, domain] = extract_user_domain(target_user);
            nego.set_identity(username.c_str(), domain.c_str(), password, "ProxyRecorder");
            // static char ln_info[] = "tsv://MS Terminal Services Plugin.1.Sessions\x0D\x0A";
            // nego.set_lb_info(byte_ptr_cast(ln_info), sizeof(ln_info)-1);
        }

        void send_negotiation_request()
        {
            this->nego.send_negotiation_request(this->trans);
        }

        bool recv_next_data(TpduBuffer& tpdu_buffer, ServerNotifier& notifier)
        {
            return this->nego.recv_next_data(
                tpdu_buffer, this->trans,
                RdpNego::ServerCert{
                    false, ServerCertCheck::always_succeed, "/tmp", notifier}
            );
        }
    };

    enum {
        NEGOCIATING_FRONT_STEP1,
        NEGOCIATING_FRONT_NLA,
        NEGOCIATING_BACK_NLA,
        NEGOCIATING_FRONT_INITIAL_PDU,
        NEGOCIATING_BACK_INITIAL_PDU,
        FORWARD
    } state = NEGOCIATING_FRONT_STEP1;

    struct TraceTransport final : SocketTransport
    {
        using SocketTransport::SocketTransport;

        Transport::Read do_atomic_read(uint8_t * buffer, std::size_t len) override
        {
            LOG_IF(enable_trace, LOG_DEBUG, "%s do_atomic_read", name);
            return SocketTransport::do_atomic_read(buffer, len);
        }

        std::size_t do_partial_read(uint8_t * buffer, std::size_t len) override
        {
            LOG_IF(enable_trace, LOG_DEBUG, "%s do_partial_read", name);
            return SocketTransport::do_partial_read(buffer, len);
        }

        void do_send(const uint8_t * buffer, std::size_t len) override
        {
            LOG_IF(enable_trace, LOG_DEBUG, "%s do_send", name);
            SocketTransport::do_send(buffer, len);
        }

        bool enable_trace = false;
    };

    TraceTransport frontConn;
    TraceTransport backConn;

    RecorderFile outFile;

    uint8_t tmpBuffer[0xffff];

    TpduBuffer frontBuffer;
    TpduBuffer backBuffer;

    std::unique_ptr<NegoClient> nego_client;
    std::unique_ptr<NegoServer> nego_server;

    std::string host;
    std::string nla_username;
    std::string nla_password;
    bool enable_kerberos;
    bool is_tls_client = false;
    bool is_nla_client = false;
    uint64_t verbosity;
};

/** @brief the server that handles RDP connections */
class FrontServer : public Server
{
public:
    FrontServer(std::string host, int port, std::string captureFile, std::string nla_username, std::string nla_password, bool enable_kerberos, uint64_t verbosity)
        : targetPort(port)
        , targetHost(std::move(host))
        , captureTemplate(std::move(captureFile))
        , nla_username(std::move(nla_username))
        , nla_password(std::move(nla_password))
        , enable_kerberos(enable_kerberos)
        , verbosity(verbosity)
    {
        // just ignore this signal because there is no child termination management yet.
        struct sigaction sa;
        sa.sa_flags = 0;
        sigaddset(&sa.sa_mask, SIGCHLD);
        sa.sa_handler = SIG_IGN; /*NOLINT*/
        sigaction(SIGCHLD, &sa, nullptr);
    }

    Server::Server_status start(int sck, bool forkable) override {
        unique_fd sck_in {accept(sck, nullptr, nullptr)};
        if (!sck_in) {
            LOG(LOG_ERR, "Accept failed on socket %d (%s)", sck, strerror(errno));
            _exit(1);
        }

        const pid_t pid = forkable ? fork() : 0;
        connection_counter++;

        if(pid == 0) {
            close(sck);

            int nodelay = 1;
            if (setsockopt(sck_in.fd(), IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay)) < 0) {
                LOG(LOG_ERR, "Failed to set socket TCP_NODELAY option on client socket");
                _exit(1);
            }

            char finalPathBuffer[256];
            char const* finalPath = captureTemplate.format(finalPathBuffer, connection_counter);
            LOG(LOG_INFO, "Recording front connection in %s", finalPath);

            TimeSystem timeobj;
            FrontConnection conn(
                std::move(sck_in), targetHost, targetPort, finalPath, timeobj,
                nla_username, nla_password, enable_kerberos, verbosity);
            try {
                conn.run();
            } catch(Error const& e) {
                if (errno) {
                    LOG(LOG_ERR, "Recording front connection ending: %s ; %s", e.errmsg(), strerror(errno));
                }
                else {
                    LOG(LOG_ERR, "Recording front connection ending: %s", e.errmsg());
                }
                exit(1);
            }
            exit(0);
        }
        else if (!forkable) {
            return Server::START_FAILED;
        }

        return Server::START_OK;
    }

private:
    struct CaptureTemplate
    {
        std::string captureTemplate;
        int startDigit = 0;
        int endDigit = 0;

        CaptureTemplate(std::string captureFile)
            : captureTemplate(std::move(captureFile))
        {
            std::string::size_type pos = 0;
            while ((pos = captureTemplate.find('%', pos)) != std::string::npos) {
                if (captureTemplate[pos+1] == 'd') {
                    startDigit = int(pos);
                    endDigit = startDigit + 2;
                    break;
                }
                ++pos;
            }
        }

        char const* format(array_view_char path, int counter) const
        {
            if (endDigit) {
                std::snprintf(path.data(), path.size(), "%.*s%04d%s",
                    startDigit, captureTemplate.c_str(),
                    counter,
                    captureTemplate.c_str() + endDigit
                );
                path.back() = '\0';
                return path.data();
            }

            return captureTemplate.c_str();
        }
    };

    int connection_counter = 0;
    int targetPort;
    std::string targetHost;
    CaptureTemplate captureTemplate;
    std::string nla_username;
    std::string nla_password;
    bool enable_kerberos;
    uint64_t verbosity;
};


struct CliPassword
{
    cli::Res operator()(cli::ParseResult& pr) const
    {
        if (!pr.str) {
            return cli::Res::BadFormat;
        }

        char* s = av[pr.opti];
        password = s;
        // hide password in /proc/...
        for (int i = 0; *s; ++s, ++i) {
            *s = (i < 3) ? '*' : '\0';
        }
        ++pr.opti;

        return cli::Res::Ok;
    }

    std::string& password;
    char ** av;
};

template<class Output, class Opt>
void print_action(Output&& out, Opt const& /*unused*/, CliPassword const& /*clipass*/)
{
    out << " [password]";
}

int main(int argc, char *argv[])
{
    char const* target_host = nullptr;
    int target_port = 3389;
    int listen_port = 3389;
    char const* capture_file = nullptr;
    std::string nla_username;
    std::string nla_password;
    bool no_forkable = false;
    bool enable_kerberos = false;
    uint64_t verbosity = 0;

    auto options = cli::options(
        cli::option('h', "help").help("Show help").action(cli::help),
        cli::option('v', "version").help("Show version")
            .action(cli::quit([]{ std::cout << "ProxyRecorder 1.0, " << redemption_info_version() << "\n"; })),
        cli::option('s', "target-host").action(cli::arg_location("host", target_host)),
        cli::option('p', "target-port").action(cli::arg_location("port", target_port)),
        cli::option('P', "port").help("Listen port").action(cli::arg_location(listen_port)),
        cli::option("nla-username").action(cli::arg_location("username", nla_username)),
        cli::option("nla-password").action(CliPassword{nla_password, argv}),
        cli::option("enable-kerberos").action(cli::on_off_location(enable_kerberos)),
        cli::option('t', "template").help("Ex: dump-%d.out")
            .action(cli::arg_location("path", capture_file)),
        cli::option('N', "no-fork").action(cli::on_off_location(no_forkable)),
        cli::option('V', "verbose").action(cli::arg_location("verbosity", verbosity))
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
            std::cerr << "Bad " << (cli_result.res == cli::Res::BadFormat ? "format" : "option") << " at parameter " << cli_result.opti;
            if (cli_result.opti < cli_result.argc) {
                std::cerr << " (" << cli_result.argv[cli_result.opti] << ")";
            }
            std::cerr << "\n";
            return 1;
    }

    if (!target_host) {
        std::cerr << "Missing --target-host\n";
    }

    if (!capture_file) {
        std::cerr << "Missing --template\n";
    }

    if (!target_host || !capture_file) {
        cli::print_help(options, std::cerr << "\n");
        return 1;
    }

    SSL_library_init();

    openlog("FrontConnection", LOG_CONS | LOG_PERROR, LOG_USER);

    FrontServer front(
        target_host, target_port, capture_file,
        std::move(nla_username), std::move(nla_password),
        enable_kerberos, verbosity);
    Listen listener(front, inet_addr("0.0.0.0"), listen_port);
    if (listener.sck <= 0) {
        return 2;
    }
    listener.run(!no_forkable);
}
