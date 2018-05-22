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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/timerfd.h>
#include <sys/eventfd.h>
#include <fcntl.h>

#include "utils/log.hpp"
#include "utils/stream.hpp"
#include "replay_transport.hpp"
#include "recorder_transport.hpp"

ReplayTransport::ReplayTransport( const char * name, const std::string &fname, const char *ip_address
		, int port, std::chrono::milliseconds timeout, bool respect_timing, std::string * error_message)
	: respect_timing(respect_timing)
	, start_time(std::chrono::system_clock::now())
	, clock_id(CLOCK_MONOTONIC)
	, recv_timeout(timeout)
	, inFile(unique_fd(::open(fname.c_str(), O_RDONLY)) )
	, record_data(nullptr), record_ptr(nullptr)
	, public_key(nullptr), public_key_size(0)
	, record_len(0), eof(false)
{
	(void)name;
	(void)ip_address;
	(void)port;
	(void)error_message;

	if (respect_timing) {
		timer = timerfd_create(clock_id, TFD_NONBLOCK);
	} else {
		uint64_t value = 0;

		timer = eventfd(0, EFD_NONBLOCK);
		write(timer, &value, 8); // that will make the file descriptor always selectable
	}

	ReplayTransport::ReadResult res;
	do {
		res = read_more_chunk();
	} while(res == Meta);
}


ReplayTransport::~ReplayTransport() = default;


void ReplayTransport::reschedule_timer() {
	if (!respect_timing)
		return;

	itimerspec timeout = {};
	uint64_t msecDelta;
	auto now = std::chrono::system_clock::now();

	if (now >= record_time) {
		/* we passed dueTime, let's program a very short relative time*/
		msecDelta = 1;
	} else {
		msecDelta = std::max(
				std::chrono::milliseconds(1),
				std::chrono::duration_cast<std::chrono::milliseconds>(record_time - now)
		).count();
	}

	timeout.it_value.tv_sec = msecDelta / 1000;
	timeout.it_value.tv_nsec = (msecDelta % 1000) * 1000 * 1000;
	//LOG(LOG_WARNING, "trigerring timer now sec=%ld nsec=%ld", timeout.it_value.tv_sec, timeout.it_value.tv_nsec);

	if (timerfd_settime(timer, 0, &timeout, nullptr) < 0) {
		LOG(LOG_ERR, "unable to set the timer time");
	}
}

ReplayTransport::ReadResult ReplayTransport::read_more_chunk() {
	uint8_t buffer[8+4+1];
	uint32_t chunkLen, offset;
	ReplayTransport::ReadResult status = Eof;

	Transport::Read ret = inFile.atomic_read(buffer, sizeof(buffer));
	if (ret == Transport::Read::Eof) {
		eof = true;
		return Eof;
	}

	InStream instream(buffer, sizeof(buffer));

	uint8_t recordType = instream.in_uint8();
	record_time = start_time + std::chrono::milliseconds(instream.in_uint64_le());
	chunkLen = instream.in_uint32_le();

	switch (recordType) {
	case RecorderTransport::RECORD_TYPE_CERT:
		// LOG(LOG_WARNING, "cert len=%u", chunkLen);
		public_key_size = chunkLen;
		public_key = static_cast<uint8_t *>(malloc(chunkLen));
		ret = inFile.atomic_read(public_key, chunkLen);
		if (ret != Transport::Read::Ok) {
			eof = true;
			return Eof;
		}
		return Meta;

	case RecorderTransport::RECORD_TYPE_DATA_IN:
		// LOG(LOG_WARNING, "data chunk len=%u offset=%lu", chunkLen, record_offset);
		offset = record_ptr - record_data;

		record_data = record_ptr = static_cast<char *>(realloc(record_data, record_len + chunkLen));
		if (record_len)
			memmove(record_ptr, record_ptr + offset, record_len);

		ret = inFile.atomic_read(record_ptr + offset, chunkLen);
		if (ret != Transport::Read::Ok) {
			eof = true;
			return Eof;
		}
		record_len += chunkLen;
		status = Data;
		break;

	case RecorderTransport::RECORD_TYPE_EOF:
		// LOG(LOG_WARNING, "EOF !!!!!!!!!!!!!!");
		eof = true;
		status = Eof;
		break;
	}

	reschedule_timer();

	return status;
}


array_view_const_u8 ReplayTransport::get_public_key() const
{
	return {this->public_key, this->public_key_size};
}

Transport::TlsResult ReplayTransport::enable_client_tls(bool server_cert_store,
                                    ServerCertCheck server_cert_check,
                                    ServerNotifier & server_notifier,
                                    const char * certif_path)
{
	(void)server_cert_store;
	(void)server_cert_check;
	(void)server_notifier;
	(void)certif_path;

	return Transport::TlsResult::Ok;
}

size_t ReplayTransport::do_partial_read(uint8_t * buffer, size_t len) {
	size_t ret;
	uint64_t timeval;

	if (respect_timing)
		read(timer, &timeval, sizeof(timeval));

	reschedule_timer();
	if (eof) {
		throw Error(ERR_TRANSPORT_NO_MORE_DATA, 0);
	}

	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	if (record_time > now) {
		auto delta = record_time - now;
		std::chrono::milliseconds toSleep = (delta > recv_timeout) ? recv_timeout :
				std::chrono::duration_cast<std::chrono::milliseconds>(delta);
		usleep(toSleep.count() * 1000);
	}

	now = std::chrono::system_clock::now();
	if (record_time > now)
		return 0;

	if (len < record_len) {
		// partially read the record
		memcpy(buffer, record_ptr, len);
		record_ptr += len;
		record_len -= len;

		return len;
	}

	memcpy(buffer, record_ptr, record_len);
	ret = record_len;
	record_len = 0;

	ReplayTransport::ReadResult res;
	do {
		res = read_more_chunk();
		if (res == Eof) {
			eof = true;
			throw Error(ERR_TRANSPORT_NO_MORE_DATA, 0);
		}
	} while (res != Data);
	return ret;
}

Transport::Read ReplayTransport::do_atomic_read(uint8_t * buffer, size_t len) {
	std::chrono::system_clock::time_point now, dueTime;
	ReplayTransport::ReadResult res;
	uint64_t timeval;

	if (respect_timing)
		read(timer, &timeval, sizeof(timeval));
	reschedule_timer();

	if (eof) {
		return Transport::Read::Eof;
	}

	now = std::chrono::system_clock::now();
	dueTime = now + recv_timeout;

	while (now < dueTime) {
		if (now < record_time) {
			auto delta = record_time - now;
			std::chrono::milliseconds toSleep = (delta > recv_timeout) ? recv_timeout :
					std::chrono::duration_cast<std::chrono::milliseconds>(delta);
			usleep(toSleep.count() * 1000);
		}

		now = std::chrono::system_clock::now();
		if (now < record_time)
			continue;

		if (record_len <= len) {
			memcpy(buffer, record_ptr, len);
			record_len -= len;
			record_ptr += len;
			if (!record_len) {

				do {
					res = read_more_chunk();
					if (res == Eof) {
						eof = true;
						return Transport::Read::Eof;
					}
				} while (res != Data);
			}

			return Transport::Read::Ok;
		}

		do {
			res = read_more_chunk();
			if (res == Eof) {
				eof = true;
				return Transport::Read::Eof;
			}
		} while (res != Data);
	}

	throw Error(ERR_TRANSPORT_NO_MORE_DATA, 0);
}

void ReplayTransport::do_send(const uint8_t * const buffer, size_t len) {
	(void)buffer;
	(void)len;
}
