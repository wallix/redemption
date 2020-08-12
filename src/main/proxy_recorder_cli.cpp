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

#include "proxy_recorder/proxy_recorder.hpp"
#include "proxy_recorder/nla_tee_transport.hpp"
#include "system/scoped_ssl_init.hpp"
#include "transport/socket_transport.hpp"
#include "transport/socket_trace_transport.hpp"
#include "core/listen.hpp"
#include "utils/netutils.hpp"
#include "utils/select.hpp"
#include "utils/cli.hpp"
#include "utils/redemption_info_version.hpp"

#include <iostream>
#include <csignal>

#include <sys/types.h>
#include <sys/socket.h>


using PacketType = RecorderFile::PacketType;


/** @brief the server that handles RDP connections */
class FrontServer
{
public:
    FrontServer(std::string host, int port, std::string captureFile, std::string nla_username, std::string nla_password, bool enable_kerberos, bool forkable, uint64_t verbosity)
        : targetPort(port)
        , targetHost(std::move(host))
        , captureTemplate(std::move(captureFile))
        , nla_username(std::move(nla_username))
        , nla_password(std::move(nla_password))
        , enable_kerberos(enable_kerberos)
        , forkable(forkable)
        , verbosity(verbosity)
    {
        // just ignore this signal because there is no child termination management yet.
        struct sigaction sa;
        sa.sa_flags = 0;
        sigaddset(&sa.sa_mask, SIGCHLD);
        sa.sa_handler = SIG_IGN; /*NOLINT*/
        sigaction(SIGCHLD, &sa, nullptr);
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

//            int nodelay = 1;
//            if (setsockopt(sck_in.fd(), IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay)) < 0) {
//                LOG(LOG_ERR, "Failed to set socket TCP_NODELAY option on client socket");
//                _exit(1);
//            }

            char finalPathBuffer[256];
            char const* finalPath = captureTemplate.format(
                make_writable_array_view(finalPathBuffer), connection_counter);
            LOG(LOG_INFO, "Recording front connection in %s", finalPath);
            TimeSystem timeobj;
            RecorderFile outFile(timeobj, finalPath);

            SocketTransport lowFrontConn("front", std::move(sck_in), "127.0.0.1", 3389, std::chrono::milliseconds(100), to_verbose_flags(verbosity));
            SocketTransport lowBackConn("back", ip_connect(this->targetHost.c_str(), this->targetPort),
                this->targetHost.c_str(), this->targetPort, std::chrono::milliseconds(100), to_verbose_flags(verbosity));
            TraceTransport frontConn("front", lowFrontConn);
            TraceTransport backConn("back", lowBackConn);
            NlaTeeTransport front_nla_tee_trans(frontConn, outFile, NlaTeeTransport::Type::Server);
            NlaTeeTransport back_nla_tee_trans(backConn, outFile, NlaTeeTransport::Type::Client);

            ProxyRecorder conn(back_nla_tee_trans, outFile, timeobj, this->targetHost.c_str(), enable_kerberos, verbosity);

            try {
                // TODO: key becomes ready quite late (just before calling nego server) inside front_step1(), henceforth doing it here won't work
                // but as it is an array view the adress of the array that will contain the key is already ok
                // henceforth it should work anyway... put the call in the right place after inlining conn.run() and splitting front_step1()
                uint8_t front_public_key[1024] = {};
                writable_u8_array_view front_public_key_av;
                // TODO: move run() code here (as it is not testable putting it in ProxyRecorder instance is troublesome)

                fd_set rset;
                int const front_fd = frontConn.get_fd();
                int const back_fd = backConn.get_fd();

                frontConn.set_trace_send(conn.verbosity > 512);
                backConn.set_trace_send(conn.verbosity > 512);

                for (;;) {
                    io_fd_zero(rset);
                    switch(conn.pstate) {
                    case ProxyRecorder::PState::NEGOCIATING_FRONT_NLA:
                    case ProxyRecorder::PState::NEGOCIATING_FRONT_STEP1:
                    case ProxyRecorder::PState::NEGOCIATING_FRONT_INITIAL_PDU:
                        // Negotiation with back delayed until front finished
                        io_fd_set(front_fd, rset);
                        break;
                    case ProxyRecorder::PState::NEGOCIATING_BACK_NLA:
                    case ProxyRecorder::PState::NEGOCIATING_BACK_INITIAL_PDU:
                        // Now start negociation with back
                        // FIXME: use front NLA parameters!
                        io_fd_set(back_fd, rset);
                        break;
                    case ProxyRecorder::PState::FORWARD:
                        io_fd_set(front_fd, rset);
                        io_fd_set(back_fd, rset);
                        break;
                    }

                    int status = select(std::max(front_fd, back_fd) + 1, &rset, nullptr, nullptr, nullptr);
                    if (status < 0) {
                        break;
                    }

                    switch(conn.pstate) {
                    case ProxyRecorder::PState::NEGOCIATING_FRONT_STEP1:
                        if (FD_ISSET(front_fd, &rset)) {
                            conn.frontBuffer.load_data(frontConn);
                            if (conn.frontBuffer.next(TpduBuffer::PDU)) {
                                conn.front_step1(frontConn);
                                u8_array_view key = front_nla_tee_trans.get_public_key();
                                memcpy(front_public_key, key.data(), key.size());
                                front_public_key_av = writable_array_view(front_public_key, key.size());
                                conn.back_step1(front_public_key_av, backConn, this->nla_username, this->nla_password);
                            }
                        }
                        break;

                    case ProxyRecorder::PState::NEGOCIATING_FRONT_NLA:
                        if (FD_ISSET(front_fd, &rset)) {
                            conn.frontBuffer.load_data(frontConn);
                            conn.front_nla(frontConn);
                        }
                        break;

                    case ProxyRecorder::PState::NEGOCIATING_FRONT_INITIAL_PDU:
                        if (FD_ISSET(front_fd, &rset)) {
                            conn.frontBuffer.load_data(frontConn);
                            conn.front_initial_pdu_negociation(backConn, !this->nla_username.empty());
                        }
                        break;

                    case ProxyRecorder::PState::NEGOCIATING_BACK_NLA:
                        if (FD_ISSET(back_fd, &rset)) {
                            conn.back_nla_negociation(backConn);
                        }
                        break;

                    case ProxyRecorder::PState::NEGOCIATING_BACK_INITIAL_PDU:
                        // Now start negociation with back
                        // FIXME: use front NLA parameters!
                        if (FD_ISSET(back_fd, &rset)) {
                            conn.backBuffer.load_data(backConn);
                            conn.back_initial_pdu_negociation(frontConn, !this->nla_username.empty());
                        }
                        break;

                    case ProxyRecorder::PState::FORWARD:
                        if (FD_ISSET(front_fd, &rset)) {
                            frontConn.set_trace_receive(conn.verbosity > 1024);
                            backConn.set_trace_send(conn.verbosity > 1024);
                            LOG_IF(conn.verbosity > 1024, LOG_INFO, "FORWARD (FRONT TO BACK)");
                            uint8_t tmpBuffer[0xffff];
                            size_t ret = frontConn.partial_read(make_writable_array_view(tmpBuffer));
                            if (ret > 0) {
                                outFile.write_packet(PacketType::DataOut, {tmpBuffer, ret});
                                backConn.send(tmpBuffer, ret);
                            }
                        }
                        if (FD_ISSET(back_fd, &rset)) {
                            frontConn.set_trace_send(conn.verbosity > 1024);
                            backConn.set_trace_receive(conn.verbosity > 1024);
                            LOG_IF(conn.verbosity > 1024, LOG_INFO, "FORWARD (BACK to FRONT)");
                            uint8_t tmpBuffer[0xffff];
                            size_t ret = backConn.partial_read(make_writable_array_view(tmpBuffer));
                            if (ret > 0) {
                                frontConn.send(tmpBuffer, ret);
                                outFile.write_packet(PacketType::DataIn, {tmpBuffer, ret});
                            }
                        }
                        break;
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

        char const* format(writable_chars_view path, int counter) const
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
    bool forkable;
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
    int listen_port = 8001;
    std::string nla_username;
    std::string nla_password;
    char const* capture_file = nullptr;
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

    ScopedSslInit scoped_ssl;

    openlog("ProxyRecorder", LOG_CONS | LOG_PERROR, LOG_USER);

    FrontServer front(
        target_host, target_port, capture_file,
        std::move(nla_username), std::move(nla_password),
        enable_kerberos, !no_forkable, verbosity);
    auto sck = create_server(inet_addr("0.0.0.0"), listen_port, EnableTransparentMode::No);
    if (!sck) {
        return 2;
    }
    return unique_server_loop(std::move(sck), [&](int sck){
        return front.start(sck);
    });
}
