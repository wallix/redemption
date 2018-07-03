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
#include <netinet/tcp.h>
#include <sys/select.h>

#include <chrono>

#include <core/listen.hpp>
#include <core/session_reactor.hpp>
#include <core/RDP/tpdu_buffer.hpp>
//#include <core/RDP/nego.hpp>
#include <core/server_notifier_api.hpp>

#include <transport/socket_transport.hpp>
#include <transport/recorder_transport.hpp>
#include <utils/netutils.hpp>

#include "utils/fixed_random.hpp"

using PacketType = RecorderFile::PacketType;

/** @brief a front connection with a RDP client */
class FrontConnection {
public:
	FrontConnection(int sck, int connId, const char *host, int port, const std::string &captureFile)
		: frontConn("front", unique_fd(sck), "127.0.0.1", 3389, std::chrono::milliseconds(100), to_verbose_flags(0))
		, frontProtocols(0)
		, backConn("back", ip_connect(host, port, 3, 1000), host, port, std::chrono::milliseconds(100), to_verbose_flags(0))
		, captureFile(captureFile)
		, conn_no(connId)
		, tmpBuffer(new uint8_t[0xffff])
		, state(NEGOCIATING_FRONT_STEP1)
		, outFile(captureFile.c_str())
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
			size_t ret = frontConn.do_partial_read(tmpBuffer, 0xffff);
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
			size_t ret = backConn.do_partial_read(tmpBuffer, 0xffff);
			if (ret > 0) {
				frontConn.send(tmpBuffer, ret);
				outFile.write_packet(PacketType::DataIn, {tmpBuffer, ret});
			}
			break;
		}
	}


	void run() {
		fd_set rset;
		bool doRun = true;

		LOG(LOG_ERR, "Recording front connection in %s", captureFile.c_str());
		int front_fd = frontConn.get_fd();
		int back_fd = backConn.get_fd();

		while (doRun) {
			int status;

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

			status = select(std::max(front_fd, back_fd) + 1, &rset, nullptr, nullptr, nullptr);
			if (status < 0)
				break;

			try {
				if (FD_ISSET(front_fd, &rset)) {
					treat_front_activity();
				}

				if (FD_ISSET(back_fd, &rset)) {
					treat_back_activity();
				}
			} catch(...) {
				doRun = false;
			}
		}
	}

protected:
	FixedRandom random;
	SocketTransport frontConn;
	TpduBuffer frontBuffer;
	uint32_t frontProtocols;

	SocketTransport backConn;
	TpduBuffer backBuffer;

	std::string captureFile;
	int conn_no;
	uint8_t *tmpBuffer;

	enum {
		NEGOCIATING_FRONT_STEP1,
		NEGOCIATING_BACK_STEP1,
		NEGOCIATING_BACK_STEP2,
		FORWARD
	} state;

	RecorderFile outFile;
};

/** @brief the server that handles RDP connections */
class FrontServer : public Server {
public:
	FrontServer(const std::string &host, int port, const std::string &captureFile)
		: connection_counter(0)
		, targetHost(host)
		, targetPort(port)
		, captureTemplate(captureFile)
	{
	}

	Server::Server_status start(int sck, bool forkable) override {
		(void)forkable;

		int sck_in = accept(sck, nullptr, nullptr);
		if (-1 == sck_in) {
			LOG(LOG_INFO, "Accept failed on socket %d (%s)", sck, strerror(errno));
			_exit(1);
		}

        const pid_t pid = fork();
        connection_counter++;

        if(pid == 0) {
        	openlog("FrontConnection", LOG_CONS | LOG_PERROR, LOG_USER);
			close(sck);

			int nodelay = 1;
			if (setsockopt(sck_in, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay)) < 0) {
				LOG(LOG_ERR, "Failed to set socket TCP_NODELAY option on client socket");
				_exit(1);
			}

			char finalPath[300];
			std::snprintf(finalPath, sizeof(finalPath), captureTemplate.c_str(), connection_counter);

			FrontConnection conn(sck_in, connection_counter, targetHost.c_str(), targetPort, std::string(finalPath));
			conn.run();
			exit(0);
		}

        close(sck_in);

    	return Server::START_OK;
    }

protected:
	int connection_counter;
	std::string targetHost;
	int targetPort;
	std::string captureTemplate;
};


int main(int argc, char *argv[]) {
	(void)argc;
	(void)argv;

	if (argc < 4) {
		LOG(LOG_ERR, "expecting target host, port and capture template");
		return 1;
	}

	FrontServer front(argv[1], strtol(argv[2], nullptr, 10), argv[3]);
	Listen listener(front, inet_addr("0.0.0.0"), 3389);
	listener.run(false);
}
