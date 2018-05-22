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

   A transport that records all the received packets
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <chrono>

#include "recorder_transport.hpp"

RecorderTransport::RecorderTransport( const char * name, const std::string &fname, unique_fd sck
		, const char *ip_address, int port, std::chrono::milliseconds recv_timeout
		, Verbose verbose, std::string * error_message)
	: SocketTransport(name, std::move(sck), ip_address, port, recv_timeout, verbose, error_message)
	, start_time(std::chrono::system_clock::now())
	, file( unique_fd(::open(fname.c_str(), O_CREAT|O_RDWR|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)) )
{
}


RecorderTransport::~RecorderTransport() = default;


Transport::TlsResult RecorderTransport::enable_client_tls(bool server_cert_store,
                                ServerCertCheck server_cert_check,
                                ServerNotifier & server_notifier,
                                const char * certif_path
    )
{
	auto ret = SocketTransport::enable_client_tls(server_cert_store, server_cert_check,
								server_notifier, certif_path);
	if (ret == Transport::TlsResult::Ok) {
		auto now = std::chrono::system_clock::now();
		auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time);
        auto const key = this->get_public_key();

		headers_stream.out_uint8(RECORD_TYPE_CERT);
		headers_stream.out_uint64_le(delta.count());
		headers_stream.out_uint32_le(key.size());

		//LOG(LOG_WARNING, "cert len=%lu", key.size());
		file.send(headers_stream.get_data(), headers_stream.get_offset());
		file.send(key.data(), key.size());
		headers_stream.rewind(0);
	}
	return ret;
}

/*void RecorderTransport::do_send(const uint8_t * const buffer, size_t len) {
	auto now = std::chrono::system_clock::now();

	headers_stream.out_uint8(RECORD_TYPE_DATA_OUT);
	headers_stream.out_uint64_le(now.time_since_epoch().count() - start_time);
	headers_stream.out_uint32_le(len);
	file.send(headers_stream.get_data(), headers_stream.get_offset());
	file.send(buffer, len);
	headers_stream.rewind(0);

	SocketTransport::do_send(buffer, len);
}*/


size_t RecorderTransport::do_partial_read(uint8_t * buffer, size_t len) {
	size_t ret = SocketTransport::do_partial_read(buffer, len);

	if (ret > 0) {
		auto now = std::chrono::system_clock::now();
		auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time);

		headers_stream.out_uint8(RECORD_TYPE_DATA_IN);
		headers_stream.out_uint64_le(delta.count());
		headers_stream.out_uint32_le(ret);
		// LOG(LOG_WARNING, "do_partial_read len=%lu", ret);

		file.send(headers_stream.get_data(), headers_stream.get_offset());
		file.send(buffer, ret);
		headers_stream.rewind(0);
	}
	return ret;
}

Transport::Read RecorderTransport::do_atomic_read(uint8_t * buffer, size_t len) {
	auto ret = SocketTransport::do_atomic_read(buffer, len);

	if (ret != Transport::Read::Eof) {
		auto now = std::chrono::system_clock::now();
		auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time);

		headers_stream.out_uint8(RECORD_TYPE_DATA_IN);
		headers_stream.out_uint64_le(delta.count());
		headers_stream.out_uint32_le(len);
		// LOG(LOG_WARNING, "do_atomic_read len=%lu", len);


		file.send(headers_stream.get_data(), headers_stream.get_offset());
		file.send(buffer, len);
		headers_stream.rewind(0);
	}

	return ret;
}


bool RecorderTransport::disconnect() {
	auto now = std::chrono::system_clock::now();
	auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time);

	headers_stream.out_uint8(RECORD_TYPE_EOF);
	headers_stream.out_uint64_le(delta.count());
	headers_stream.out_uint32_le(0);

	file.close();
	return SocketTransport::disconnect();
}
