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

#include "core/RDP/tpdu_buffer.hpp"
//#include "core/RDP/nego.hpp"
#include "core/listen.hpp"
#include "core/server_notifier_api.hpp"
#include "core/session_reactor.hpp"
#include "main/version.hpp"
#include "transport/recorder_transport.hpp"
#include "transport/socket_transport.hpp"
#include "utils/cli.hpp"
#include "utils/fixed_random.hpp"
#include "utils/netutils.hpp"

#include <chrono>
#include <iostream>

#include <netinet/tcp.h>
#include <sys/select.h>
#include <openssl/ssl.h>


using PacketType = RecorderFile::PacketType;

/** @brief a front connection with a RDP client */
class FrontConnection
{
public:
    FrontConnection(unique_fd sck, std::string const& host, int port, std::string captureFile)
        : frontConn("front", std::move(sck), "127.0.0.1", 3389, std::chrono::milliseconds(100), to_verbose_flags(0))
        , backConn("back", ip_connect(host.c_str(), port, 3, 1000),
            host.c_str(), port, std::chrono::milliseconds(100), to_verbose_flags(0))
        , captureFile(std::move(captureFile))
        , outFile(this->captureFile.c_str())
    {
    }

    void treat_front_activity() {
        switch (state) {
        case NEGOCIATING_FRONT_STEP1:
            frontBuffer.load_data(this->frontConn);
            if (frontBuffer.next_pdu()) {
                array_view_u8 currentPacket = frontBuffer.current_pdu_buffer();
                outFile.write_packet(PacketType::DataOut, currentPacket);

                InStream new_x224_stream(currentPacket);
                X224::CR_TPDU_Recv x224(new_x224_stream, true);
                if (x224._header_size != new_x224_stream.get_capacity()) {
                    LOG(LOG_WARNING, "Front::incoming: connection request : all data should have been consumed,"
                                 " %zu bytes remains", new_x224_stream.get_capacity() - x224._header_size);
                }

                backConn.send(currentPacket.data(), currentPacket.size());

                state = NEGOCIATING_BACK_STEP1;
            }
            break;
        default: // FRONT_FORWARD
            size_t ret = frontConn.partial_read(make_array_view(tmpBuffer));
            if (ret > 0) {
                outFile.write_packet(PacketType::DataOut, {tmpBuffer, ret});
                backConn.send(tmpBuffer, ret);
            }
            break;
        }
    }

    void treat_back_activity() {
        NullServerNotifier null_notifier;

        switch (state) {
        case NEGOCIATING_BACK_STEP1:
            backBuffer.load_data(this->backConn);
            if (backBuffer.next_pdu()) {
                array_view_u8 currentPacket = backBuffer.current_pdu_buffer();
                outFile.write_packet(PacketType::DataIn, currentPacket);

                InStream x224_stream(currentPacket);
                X224::CC_TPDU_Recv x224(x224_stream);

                StaticOutStream<65536> stream;
                if (x224.rdp_neg_type == X224::RDP_NEG_NONE) {
                    LOG(LOG_INFO, "RdpNego::recv_connection_confirm done (legacy, no TLS)");
                }

                /* forward the answer as is */
                frontConn.send(currentPacket.data(), currentPacket.size());


                switch (x224.rdp_neg_code) {
                case X224::PROTOCOL_TLS:
                case X224::PROTOCOL_HYBRID:
                case X224::PROTOCOL_HYBRID_EX: {
                    frontConn.enable_server_tls("inquisition", nullptr);

                    switch(backConn.enable_client_tls(false, ServerCertCheck::always_succeed, null_notifier, "/tmp")) {
                    case Transport::TlsResult::Ok:
                        outFile.write_packet(PacketType::ClientCert, backConn.get_public_key());
                        break;
                    case Transport::TlsResult::Fail:
                        break;
                    case Transport::TlsResult::Want:
                        break;
                    }
                    break;
                }
                default: /* X224::PROTOCOL_RDP */
                    break;
                }

                state = FORWARD;
            }
            break;

        default:
            size_t ret = backConn.partial_read(make_array_view(tmpBuffer));
            if (ret > 0) {
                frontConn.send(tmpBuffer, ret);
                outFile.write_packet(PacketType::DataIn, {tmpBuffer, ret});
            }
            break;
        }
    }


    void run() {
        fd_set rset;
        int front_fd = frontConn.get_fd();
        int back_fd = backConn.get_fd();

        try {
            for (;;) {
                FD_ZERO(&rset);

                switch(state) {
                case NEGOCIATING_FRONT_STEP1:
                    FD_SET(front_fd, &rset);
                    break;
                case NEGOCIATING_BACK_STEP1:
                case NEGOCIATING_BACK_STEP2:
                    FD_SET(back_fd, &rset);
                    break;
                default:
                    FD_SET(front_fd, &rset);
                    FD_SET(back_fd, &rset);
                    break;
                }

                int status = select(std::max(front_fd, back_fd) + 1, &rset, nullptr, nullptr, nullptr);
                if (status < 0) {
                    LOG(LOG_ERR, "select error: %s [%d]", strerror(errno), errno);
                    break;
                }

                if (FD_ISSET(front_fd, &rset)) {
                    treat_front_activity();
                }

                if (FD_ISSET(back_fd, &rset)) {
                    treat_back_activity();
                }
            }
        } catch(Error const& e) {
            LOG(LOG_ERR, "Recording front connection ending: %s", e.errmsg());
        }
    }

private:
    FixedRandom random;
    SocketTransport frontConn;
    TpduBuffer frontBuffer;

    SocketTransport backConn;
    TpduBuffer backBuffer;

    std::string captureFile;
    uint8_t tmpBuffer[0xffff];

    enum {
        NEGOCIATING_FRONT_STEP1,
        NEGOCIATING_BACK_STEP1,
        NEGOCIATING_BACK_STEP2,
        FORWARD
    } state = NEGOCIATING_FRONT_STEP1;

    RecorderFile outFile;
};

/** @brief the server that handles RDP connections */
class FrontServer : public Server
{
public:
    FrontServer(std::string host, int port, std::string captureFile)
        : targetPort(port)
        , targetHost(std::move(host))
        , captureTemplate(std::move(captureFile))
    {
    }

    Server::Server_status start(int sck, bool /*forkable*/) override {
        unique_fd sck_in {accept(sck, nullptr, nullptr)};
        if (!sck_in) {
            LOG(LOG_INFO, "Accept failed on socket %d (%s)", sck, strerror(errno));
            _exit(1);
        }

        const pid_t pid = fork();
        connection_counter++;

        if(pid == 0) {
            openlog("FrontConnection", LOG_CONS | LOG_PERROR, LOG_USER);
            close(sck);

            int nodelay = 1;
            if (setsockopt(sck_in.fd(), IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay)) < 0) {
                LOG(LOG_ERR, "Failed to set socket TCP_NODELAY option on client socket");
                _exit(1);
            }

            char finalPathBuffer[256];
            char const* finalPath = captureTemplate.format(finalPathBuffer, connection_counter);
            LOG(LOG_INFO, "Recording front connection in %s", finalPath);

            FrontConnection conn(std::move(sck_in), targetHost, targetPort, finalPath);
            conn.run();
            exit(0);
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
                LOG(LOG_INFO, "%lu", pos);
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
};


int main(int argc, char *argv[])
{
    char const* target_host = nullptr;
    int target_port = 3389;
    int listen_port = 3389;
    char const* capture_file = nullptr;

    auto options = cli::options(
        cli::option('h', "help").help("Show help").action(cli::help),
        cli::option('v', "version").help("Show version")
            .action(cli::quit([]{ std::cout << "Version 1, ReDemPtion " VERSION << "\n"; })),
        cli::option('s', "target-host").action(cli::arg_location("host", target_host)),
        cli::option('p', "target-port").action(cli::arg_location("port", target_port)),
        cli::option('P', "port").help("Listen port").action(cli::arg_location(listen_port)),
        cli::option('t', "template").help("Ex: dump-%d.out")
            .action(cli::arg_location("path", capture_file))
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

    FrontServer front(target_host, target_port, capture_file);
    Listen listener(front, inet_addr("0.0.0.0"), listen_port);
    listener.run(false);
}
