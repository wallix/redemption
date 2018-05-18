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
*/

#pragma once

#include <chrono>
#include "transport/transport.hpp"
#include "transport/in_file_transport.hpp"
#include "utils/sugar/unique_fd.hpp"

/**
 *	@brief a transport that will replay a full capture
 */
class ReplayTransport : public Transport {
public:
	ReplayTransport( const char *name, const std::string &fname, const char *ip_address, int port
					   , std::chrono::milliseconds recv_timeout, bool respect_timing
					   , std::string * error_message = nullptr);

	~ReplayTransport() override;

    virtual const uint8_t * get_public_key() const override;

    virtual size_t get_public_key_length() const override;

    TlsResult enable_client_tls(bool server_cert_store,
                                    ServerCertCheck server_cert_check,
                                    ServerNotifier & server_notifier,
                                    const char * certif_path) override;

    size_t do_partial_read(uint8_t * buffer, size_t len) override;

    Read do_atomic_read(uint8_t * buffer, size_t len) override;

    void do_send(const uint8_t * const buffer, size_t len) override;

    int get_fd() const override { return timer; }

protected:
    /** @brief the result of read_more_chunk */
    enum ReadResult { Data, Meta, Eof };

    ReadResult read_more_chunk();

    void reschedule_timer();

protected:
    bool respect_timing;
    std::chrono::system_clock::time_point start_time;
    int clock_id;
    std::chrono::milliseconds recv_timeout;
    InFileTransport inFile;
	int timer;

	std::chrono::system_clock::time_point record_time;
	char *record_data, *record_ptr;
	uint8_t *public_key;
	size_t public_key_size;
	uint32_t record_len;
	bool eof;
};
