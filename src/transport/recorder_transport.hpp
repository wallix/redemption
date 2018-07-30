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

*/

#pragma once

#include "transport/transport.hpp"
#include "transport/out_file_transport.hpp"

#include <chrono>

/**
 * @brief a file containing a capture
 */
class RecorderFile {
public:
	/** @brief type of packet */
	enum class PacketType : uint8_t
	{
		DataIn,
		DataOut,
		ClientCert,
		ServerCert,
		Eof,
		Disconnect,
		Connect,
		Info,
		NlaIn,
		NlaOut,
	};

	explicit RecorderFile(char const* filename);

	~RecorderFile();

	void write_packet(PacketType type, const_byte_array buffer);

protected:
    std::chrono::time_point<std::chrono::system_clock> start_time;
    OutFileTransport file;
};


/**
 * @brief a socket transport that records all the sent packets
 */
class RecorderTransport : public Transport
{
public:

	explicit RecorderTransport(Transport& trans, char const* filename);

	void add_info(byte_array info);

    TlsResult enable_client_tls(
        bool server_cert_store, ServerCertCheck server_cert_check,
        ServerNotifier & server_notifier, const char * certif_path
    ) override;

    void enable_server_tls(const char * certificate_password, const char * ssl_cipher_list) override;

    array_view_const_u8 get_public_key() const override;

    void flush() override;

    bool disconnect() override;

    bool connect() override;

    void timestamp(timeval now) override;

    bool next() override;

    int get_fd() const override;

private:
    Read do_atomic_read(uint8_t * buffer, size_t len) override;

    size_t do_partial_read(uint8_t * buffer, size_t len) override;

    void do_send(const uint8_t * buffer, size_t len) override;

private:
    Transport& trans;
    RecorderFile out;
};

/**
 * @brief header of record
 */
struct RecorderTransportHeader
{
    RecorderFile::PacketType type;
    std::chrono::milliseconds record_duration;
    uint32_t data_size;
};

RecorderTransportHeader read_recorder_transport_header(Transport& trans);
