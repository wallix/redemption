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
   Copyright (C) Wallix 2013
   Author(s): David Fort

   XXXXXXXXXXXXXX
*/

#pragma once

#include "transport/socket_transport.hpp"
#include "transport/out_file_transport.hpp"
#include "utils/stream.hpp"

#include <chrono>

/**
 * @brief a socket transport that records all the sent packets
 */
class RecorderTransport : public SocketTransport {
public:
	enum PacketType : uint8_t {
		RECORD_TYPE_DATA_IN,
		RECORD_TYPE_DATA_OUT,
		RECORD_TYPE_CERT,
		RECORD_TYPE_EOF
	};
public:
	RecorderTransport( const char * name, const std::string &fname, unique_fd sck, const char *ip_address, int port
					   , std::chrono::milliseconds recv_timeout
					   , Verbose verbose, std::string * error_message = nullptr);

	~RecorderTransport() override;

	/* void do_send(const uint8_t * const buffer, size_t len) override;*/

	TlsResult enable_client_tls(bool server_cert_store,
                                ServerCertCheck server_cert_check,
                                ServerNotifier & server_notifier,
                                const char * certif_path
    ) override;

    size_t do_partial_read(uint8_t * buffer, size_t len) override;

    Read do_atomic_read(uint8_t * buffer, size_t len) override;

    bool disconnect() override;

protected:

    std::chrono::time_point<std::chrono::system_clock> start_time;
	OutFileTransport file;
	StaticOutStream<100> headers_stream;
};
