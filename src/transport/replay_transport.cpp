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

#include "utils/log.hpp"
#include "utils/hexdump.hpp"
#include "utils/stream.hpp"
#include "replay_transport.hpp"
#include "recorder_transport.hpp"

#include <algorithm>
#include <cstring>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/timerfd.h>
#include <sys/eventfd.h>
#include <fcntl.h>

using PacketType = RecorderFile::PacketType;

namespace
{
    unique_fd open_file(char const* filename)
    {
        unique_fd ufd{::open(filename, O_RDONLY)};
        if (!ufd) {
            throw Error(ERR_TRANSPORT_OPEN_FAILED, errno);
        }
        return ufd;
    }

    void dump_packet_differ(array_view_const_u8 data, array_view_const_u8 expected_data)
    {
        auto const p = std::mismatch(
            data.begin(), data.end(),
            expected_data.begin(), expected_data.end());
        auto pos = unsigned(p.first - data.begin());
		LOG(LOG_INFO, "At position: %u (0x%x)", pos, pos);
		LOG(LOG_INFO, "Data:");
        hexdump_av(data);
		LOG(LOG_INFO, "Expected:");
        hexdump_av(expected_data);
    }

    #ifdef DEBUG_PACKETS
    const char *PacketTypeToString(PacketType t) noexcept
    {
        # define CASE(name) case PacketType::name: return #name
        switch(t) {
            CASE(DataIn);
            CASE(DataOut);
            CASE(ClientCert);
            CASE(ServerCert);
            CASE(Eof);
            CASE(Disconnect);
            CASE(Connect);
            CASE(Info);
            CASE(NlaClientIn);
            CASE(NlaClientOut);
            CASE(NlaServerIn);
            CASE(NlaServerOut);
        }
        # undef
        return "unknown";
    }
    #endif
} // namespace

ReplayTransport::ReplayTransport(
    const char* fname, const char *ip_address, int port,
    FdType fd_type, FirstPacket first_packet, UncheckedPacket unchecked_packet)
: start_time(std::chrono::system_clock::now())
, in_file(open_file(fname))
, fd(FdType::Timer == fd_type
? timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK)
: []{
    int fd = eventfd(0, EFD_NONBLOCK);
    if (fd < 0) {
        throw Error(ERR_TRANSPORT_OPEN_FAILED, errno);
    }
    uint64_t value = 1;
    [[maybe_unused]] auto ret = write(fd, &value, 8); // that will make the file descriptor always selectable
    return fd;
}())
, fd_type(fd_type)
, fd_current_type(
    (first_packet == FirstPacket::EnableTimer && FdType::Timer == fd_type)
    ? FdType::Timer : FdType::AlwaysReady)
, unchecked_packet(unchecked_packet)
, data_in_pos(0)
, data_out_pos(0)
{
    (void)ip_address;
    (void)port;

    if (FdType::Timer == fd_type) {
        if (FirstPacket::EnableTimer == first_packet) {
            reschedule_timer();
        }
        else {
            prefetchForTimer();
            itimerspec timeout {};
            timeout.it_value.tv_sec = 10;
            if (timerfd_settime(this->fd.fd(), 0, &timeout, nullptr) < 0) {
                LOG(LOG_ERR, "unable to set the timer time");
                throw Error(ERR_TRANSPORT_READ_FAILED, errno);
            }
        }
    }
}


ReplayTransport::~ReplayTransport() = default;


void ReplayTransport::reschedule_timer()
{
    if (FdType::Timer != this->fd_type) {
        return;
    }

    auto targetTime = prefetchForTimer();
    auto now = std::chrono::system_clock::now();

    // zero disarms the timer, force to 1 nanoseconds
    auto const delate_time = std::max(
        std::chrono::duration_cast<std::chrono::nanoseconds>(targetTime - now),
        std::chrono::nanoseconds{1});
    auto const sec = std::chrono::duration_cast<std::chrono::seconds>(delate_time);
    auto const nano = delate_time - sec;

    itimerspec timeout = {};
    timeout.it_value.tv_sec = sec.count();
    timeout.it_value.tv_nsec = nano.count();

    LOG(LOG_INFO, "scheduling in sec=%ld nsec=%ld", timeout.it_value.tv_sec, timeout.it_value.tv_nsec);

    if (timerfd_settime(this->fd.fd(), 0, &timeout, nullptr) < 0) {
        LOG(LOG_ERR, "unable to set the timer time");
        throw Error(ERR_TRANSPORT_READ_FAILED, errno);
    }
}


ReplayTransport::Data *ReplayTransport::read_single_chunk()
{
	for (;;) {
		RecorderTransportHeader const header = read_recorder_transport_header(this->in_file);

		switch (header.type) {
		case PacketType::Info:
			this->infos.emplace_back();
			this->infos.back().resize(header.data_size);
			this->in_file.recv_boom(this->infos.back());
			continue;

		case PacketType::NlaClientIn:
		case PacketType::NlaClientOut:
		case PacketType::NlaServerIn:
		case PacketType::NlaServerOut:
            this->in_file.recv_boom(
                std::make_unique<uint8_t[]>(header.data_size).get(),
                header.data_size);
			continue;

		case PacketType::DataIn:
		case PacketType::DataOut:
		case PacketType::ClientCert:
		case PacketType::ServerCert:
		case PacketType::Eof:
		case PacketType::Disconnect:
		case PacketType::Connect:
            ReplayTransport::Data *ret = &mPrefetchQueue.emplace_back();
            ret->type = header.type;
            ret->time = this->start_time + header.record_duration;
            ret->data = std::make_unique<uint8_t[]>(header.data_size);
            ret->size = header.data_size;
            this->in_file.recv_boom(ret->data.get(), header.data_size);
            return ret;
		}

		LOG(LOG_ERR, "unknown packet type %d", header.type);
	}
}


array_view_const_u8 ReplayTransport::get_public_key() const
{
    return {this->public_key.data.get(), this->public_key.size};
}

Transport::TlsResult ReplayTransport::enable_client_tls(
    bool server_cert_store, ServerCertCheck server_cert_check,
    ServerNotifier & server_notifier, const char * certif_path)
{
    (void)server_cert_store;
    (void)server_cert_check;
    (void)server_notifier;
    (void)certif_path;

    size_t cert_pos;

    try {
    	cert_pos = searchAndPrefetchFor(PacketType::ClientCert);
    } catch(...) {
		throw Error(ERR_TRANSPORT_NO_MORE_DATA);
    }

	if (cert_pos != 0) {
		LOG(LOG_ERR, "ReplayTransport::enable_client_tls: enabling TLS is a synchronization point, so we should not have untreated records [pck_num=%lld]", this->count_packet);
		throw Error(ERR_TRANSPORT_DIFFERS);
	}

    auto av = mPrefetchQueue[cert_pos].av();
    this->public_key.size = av.size();
    this->public_key.data = std::make_unique<uint8_t[]>(av.size());
    memcpy(this->public_key.data.get(), av.data(), av.size());

	mPrefetchQueue.erase(mPrefetchQueue.begin());

	if (data_in_pos) {
        data_in_pos--;
    }
	if (data_out_pos) {
		data_out_pos--;
    }

    reschedule_timer();
    return Transport::TlsResult::Ok;
}

void ReplayTransport::enable_server_tls(
    const char* certificate_password, const char* ssl_cipher_list)
{
    (void)certificate_password;
    (void)ssl_cipher_list;
}

bool ReplayTransport::connect()
{
    //this->next_current_data(PacketType::Connect);
    return true;
}

bool ReplayTransport::disconnect()
{
    //this->next_current_data(PacketType::Disconnect);
    return true;
}

array_view_const_u8 ReplayTransport::Data::av() const noexcept
{
    return {this->data.get(), this->size};
}

void ReplayTransport::read_timer()
{
    if (FdType::Timer == this->fd_type)
    {
        uint64_t timeval;
        if (sizeof(timeval) != read(this->fd.fd(), &timeval, sizeof(timeval))) {
            int const err = errno;
            LOG(LOG_ERR, "ReplayTransport::do_partial_read: read fd error");
            throw Error(ERR_TRANSPORT_NO_MORE_DATA, err);
        }
    }
}

std::chrono::system_clock::time_point ReplayTransport::prefetchForTimer() {
	/* first scan prefetch queue for something that means "select in" */
	size_t pos = data_in_pos;
	bool found = false;
	while (pos < mPrefetchQueue.size() && !found) {
		switch (mPrefetchQueue[pos].type) {
		case PacketType::DataIn:
		case PacketType::Eof:
			found = true;
			break;
		default:
			pos++;
			break;
		}
	}

	if (found) {
        return mPrefetchQueue[pos].time;
    }

	/* read records until we get something that means some dataIn (and is supposed to trigger a select()) */
	try {
		while (true) {
			Data *d = read_single_chunk();
			switch(d->type) {
			case PacketType::DataIn:
			case PacketType::Eof:
				return d->time;
			default:
				break;
			}
		}
	} catch(...) {
		/* if we've not found anything just return now so that select() will trigger right now*/
	}

	return std::chrono::system_clock::now();
}

size_t ReplayTransport::searchAndPrefetchFor(PacketType kind)
{
	++this->count_packet;
    //LOG(LOG_DEBUG, "pck_num=%lld", this->count_packet);

	size_t counter;

	switch(kind) {
	case PacketType::DataIn:
		counter = data_in_pos;
		break;
	case PacketType::DataOut:
		counter = data_out_pos;
		break;
	default:
		counter = 0;
		break;
	}

	/* try to find the requested kind of record in the prefetch queue */
	while (counter < mPrefetchQueue.size() && mPrefetchQueue[counter].type != kind) {
        ++counter;
    }

	if (counter < mPrefetchQueue.size()) {
        return counter;
    }

	/* not in the prefetch queue, let's load some records until we get the good type */
	ReplayTransport::Data *d = read_single_chunk();
	while (d->type != kind) {
		d = read_single_chunk();
	}

	return mPrefetchQueue.size() - 1u;
}

size_t ReplayTransport::do_partial_read(uint8_t * buffer, size_t const len)
{
    this->read_timer();

    size_t pos;
	try {
		pos = searchAndPrefetchFor(PacketType::DataIn);
	} catch (...) {
		throw Error(ERR_TRANSPORT_NO_MORE_DATA);
	}

	auto av = mPrefetchQueue[pos].av();
	if (av.size() > len) {
		LOG(LOG_ERR, "ReplayTransport::do_atomic_read(buf, len=%zu) should be %zu or greater [pck_num=%lld]", len, av.size(), this->count_packet);
		dump_packet_differ({buffer, len}, av);
		throw Error(ERR_TRANSPORT_DIFFERS);
	}

	memcpy(buffer, av.data(), av.size());
	data_in_pos = pos + 1;

	if (pos == 0) {
        cleanup_data(data_out_pos, PacketType::DataOut);
	}

	reschedule_timer();
	return av.size();
}

Transport::Read ReplayTransport::do_atomic_read(uint8_t * buffer, size_t len)
{
    this->read_timer();

    size_t pos;
	try {
		pos = searchAndPrefetchFor(PacketType::DataIn);
	} catch (...) {
		return Read::Eof;
	}

	auto av = mPrefetchQueue[pos].av();
	if (av.size() != len) {
		LOG(LOG_ERR, "ReplayTransport::do_atomic_read(buf, len=%zu) should be %zu or greater [pck_num=%lld]", len, av.size(), this->count_packet);
		dump_packet_differ({buffer, len}, av);
		throw Error(ERR_TRANSPORT_DIFFERS);
	}

	memcpy(buffer, av.data(), av.size());
	data_in_pos = pos + 1;

	if (pos == 0) {
        cleanup_data(data_out_pos, PacketType::DataOut);
	}

	reschedule_timer();
	return Read::Ok;
}

void ReplayTransport::do_send(const uint8_t * const buffer, size_t len)
{
	size_t pos = searchAndPrefetchFor(PacketType::DataOut);

	if (UncheckedPacket::Send == this->unchecked_packet) {
		auto av = mPrefetchQueue[pos].av();
		if (av.size() != len || 0 != memcmp(av.data(), buffer, len)) {
			if (av.size() != len) {
				LOG(LOG_ERR, "ReplayTransport::do_send(buf, len=%zu) should be %zu [pck_num=%lld]", len, av.size(), this->count_packet);
			}
			else {
				LOG(LOG_ERR, "ReplayTransport::do_send data differs [pck_num=%lld]", this->count_packet);
			}
			dump_packet_differ({buffer, len}, av);
			throw Error(ERR_TRANSPORT_DIFFERS);
		}
	}

	data_out_pos = pos + 1;

	if (pos == 0) {
        cleanup_data(data_in_pos, PacketType::DataIn);
	}

    if (fd_type != fd_current_type) {
        assert(fd_type == FdType::Timer);
        fd_current_type = fd_type;
        reschedule_timer();
    }
}

void ReplayTransport::cleanup_data(std::size_t len, PacketType type)
{
    std::size_t index = 0;
    do {
        ++index;
    } while (index < mPrefetchQueue.size() && index < len && mPrefetchQueue[index].type == type);
    /* cleanup DataIn packets that may have been already treated */
    data_in_pos -= std::min(data_in_pos, index);
    data_out_pos -= std::min(data_out_pos, index);
    mPrefetchQueue.erase(mPrefetchQueue.begin(), mPrefetchQueue.begin()+index);
}
