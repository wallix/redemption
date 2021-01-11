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

#include "transport/recorder_transport.hpp"
#include "utils/timebase.hpp"
#include "utils/difftimeval.hpp"
#include "utils/stream.hpp"


RecorderFile::RecorderFile(CRef<TimeBase> time_base, const char *filename)
    : time_base(time_base)
    , start_time(to_ms(this->time_base.get_current_time()))
    , file(unique_fd(filename, O_CREAT|O_RDWR|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH))
{
    if (!this->file.is_open()) {
        throw Error(ERR_RECORDER_FAILED_TO_OPEN_TARGET_FILE, errno);
    }
}

RecorderFile::~RecorderFile()
{
    if (this->file.is_open()) {
        this->write_packet(PacketType::Eof, nullptr);
    }
}

void RecorderFile::write_packet(PacketType type, bytes_view buffer)
{
    auto delta = to_ms(this->time_base.get_current_time()) - this->start_time;

    StaticOutStream<13> headers_stream;
    headers_stream.out_uint8(uint8_t(type));
    headers_stream.out_uint64_le(delta.count());
    headers_stream.out_uint32_le(buffer.size());
    // LOG(LOG_DEBUG, "write_packet len=%lu", len);

    this->file.send(headers_stream.get_produced_bytes());
    this->file.send(buffer);
}


RecorderTransport::RecorderTransport(Transport& trans, CRef<TimeBase> time_base, char const* filename)
    : trans(trans)
    , out(time_base, filename)
{
}


void RecorderTransport::add_info(bytes_view info)
{
    this->out.write_packet(RecorderFile::PacketType::Info, info);
}

Transport::TlsResult RecorderTransport::enable_client_tls(ServerNotifier& server_notifier, const TLSClientParams & tls_client_params)
{
    auto const r = this->trans.enable_client_tls(server_notifier, tls_client_params);
    if (r != RecorderTransport::TlsResult::Fail) {
        this->out.write_packet(RecorderFile::PacketType::ClientCert, this->trans.get_public_key());
    }
    return r;
}

void RecorderTransport::enable_server_tls(const char * certificate_password, const char * ssl_cipher_list, uint32_t tls_min_level, uint32_t tls_max_level, bool show_common_cipher_list)
{
    this->trans.enable_server_tls(certificate_password, ssl_cipher_list, tls_min_level, tls_max_level, show_common_cipher_list);
    this->out.write_packet(RecorderFile::PacketType::ServerCert, this->trans.get_public_key());
}

u8_array_view RecorderTransport::get_public_key() const
{
    return this->trans.get_public_key();
}

void RecorderTransport::flush()
{
    return this->trans.flush();
}

bool RecorderTransport::disconnect()
{
    this->out.write_packet(RecorderFile::PacketType::Disconnect, nullptr);
    return this->trans.disconnect();
}

bool RecorderTransport::connect()
{
    this->out.write_packet(RecorderFile::PacketType::Connect, nullptr);
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
    switch (r) {
        case Read::Ok: this->out.write_packet(RecorderFile::PacketType::DataIn, {buffer, len}); break;
        case Read::Eof: this->out.write_packet(RecorderFile::PacketType::Eof, {buffer, len}); break;
    }
    return r;
}

size_t RecorderTransport::do_partial_read(uint8_t * buffer, size_t len)
{
    len = this->trans.partial_read(buffer, len);
    this->out.write_packet(RecorderFile::PacketType::DataIn, {buffer, len});
    return len;
}

void RecorderTransport::do_send(const uint8_t * buffer, size_t len)
{
    this->trans.send(buffer, len);
    this->out.write_packet(RecorderFile::PacketType::DataOut, {buffer, len});
}


RecorderTransportHeader read_recorder_transport_header(Transport& trans)
{
    char data[13];
    InStream headers_stream(data);

    trans.recv_boom(make_writable_array_view(data));

    return {
        RecorderFile::PacketType(headers_stream.in_uint8()),
        std::chrono::milliseconds(headers_stream.in_uint64_le()),
        headers_stream.in_uint32_le()
    };
}
