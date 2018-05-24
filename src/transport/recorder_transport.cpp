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

RecorderTransport::RecorderTransport(Transport& trans, char const* filename)
: start_time(std::chrono::system_clock::now())
, trans(trans)
, file(unique_fd(::open(filename, O_CREAT|O_RDWR|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)))
{
    if (!this->file.is_open()) {
        throw Error(ERR_RECORDER_FAILED_TO_OPEN_TARGET_FILE, errno);
    }
}

RecorderTransport::~RecorderTransport()
{
    if (this->file.is_open()) {
        this->write_packet(PacketType::Eof, nullptr);
    }
}

Transport::TlsResult RecorderTransport::enable_client_tls(
    bool server_cert_store, ServerCertCheck server_cert_check,
    ServerNotifier & server_notifier, const char * certif_path)
{
    auto const r = this->trans.enable_client_tls(
        server_cert_store, server_cert_check, server_notifier, certif_path);
    if (r != RecorderTransport::TlsResult::Fail) {
        this->write_packet(PacketType::ClientCert, nullptr);
    }
    return r;
}

void RecorderTransport::enable_server_tls(const char * certificate_password, const char * ssl_cipher_list)
{
    this->trans.enable_server_tls(certificate_password, ssl_cipher_list);
    this->write_packet(PacketType::ServerCert, nullptr);
}

array_view_const_u8 RecorderTransport::get_public_key() const
{
    return this->trans.get_public_key();
}

void RecorderTransport::flush()
{
    return this->trans.flush();
}

bool RecorderTransport::disconnect()
{
    this->write_packet(PacketType::Disconnect, nullptr);
    return this->trans.disconnect();
}

bool RecorderTransport::connect()
{
    this->write_packet(PacketType::Connect, nullptr);
    return this->trans.connect();
}

void RecorderTransport::timestamp(timeval now)
{
    this->trans.timestamp(now);
}

bool RecorderTransport::next()
{
    return this->trans.next();
}

int RecorderTransport::get_fd() const
{
    return this->trans.get_fd();
}

Transport::Read RecorderTransport::do_atomic_read(uint8_t * buffer, size_t len)
{
    auto const r = this->trans.atomic_read(buffer, len);
    if (r == Read::Ok) {
        this->write_packet(PacketType::DataIn, {buffer, len});
    }
    return r;
}

size_t RecorderTransport::do_partial_read(uint8_t * buffer, size_t len)
{
    auto const r = this->trans.partial_read(buffer, len);
    if (r) {
        this->write_packet(PacketType::DataIn, {buffer, len});
    }
    return r;
}

void RecorderTransport::do_send(const uint8_t * buffer, size_t len)
{
    this->trans.send(buffer, len);
    this->write_packet(PacketType::DataOut, {buffer, len});
}

void RecorderTransport::write_packet(PacketType type, const_byte_array buffer)
{
	auto now = std::chrono::system_clock::now();
	auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time);

	StaticOutStream<13> headers_stream;
	headers_stream.out_uint8(uint8_t(type));
	headers_stream.out_uint64_le(delta.count());
	headers_stream.out_uint32_le(buffer.size());
    // LOG(LOG_DEBUG, "write_packet len=%lu", len);

    this->file.send(headers_stream.get_data(), headers_stream.get_offset());
    this->file.send(buffer);
}

RecorderTransportHeader read_recorder_transport_header(Transport& trans)
{
    char data[13];
	InStream headers_stream(data);

    trans.recv_boom(make_array_view(data));

	return {
        RecorderTransport::PacketType(headers_stream.in_uint8()),
        std::chrono::milliseconds(headers_stream.in_uint64_le()),
        headers_stream.in_uint32_le()
    };
}
