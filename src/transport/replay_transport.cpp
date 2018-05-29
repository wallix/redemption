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
#include "utils/stream.hpp"
#include "replay_transport.hpp"
#include "recorder_transport.hpp"

#include <cstring>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/timerfd.h>
#include <sys/eventfd.h>
#include <fcntl.h>


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
}

ReplayTransport::ReplayTransport(
    const char* fname, const char *ip_address, int port, Timing timing)
: start_time(std::chrono::system_clock::now())
, in_file(open_file(fname))
, timer_fd(Timing::Real == timing ? timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK) : -1)
, timing(timing)
{
    (void)ip_address;
    (void)port;

    if (Timing::Real == timing && !this->timer_fd) {
        throw Error(ERR_TRANSPORT_OPEN_FAILED, errno);
    }

    this->read_more_chunk();
    this->reschedule_timer();
}


ReplayTransport::~ReplayTransport() = default;


void ReplayTransport::reschedule_timer()
{
    if (Timing::Real != this->timing) {
        return;
    }

    auto const now = std::chrono::system_clock::now();
    auto const delate_time = (this->record_time - now);

    itimerspec timeout = {};
    if (delate_time.count() > 0) {
        auto sec = std::chrono::duration_cast<std::chrono::seconds>(delate_time);
        auto nano = std::chrono::duration_cast<std::chrono::nanoseconds>(delate_time - sec);
        timeout.it_value.tv_sec = sec.count();
        timeout.it_value.tv_nsec = nano.count();
    }
    // zero disarms the timer, force to 1 nanoseconds
    if (!timeout.it_value.tv_nsec && !timeout.it_value.tv_nsec) {
        timeout.it_value.tv_nsec = 1;
    }

    // LOG(LOG_DEBUG, "trigerring timer now sec=%ld nsec=%ld", timeout.it_value.tv_sec, timeout.it_value.tv_nsec);

    if (timerfd_settime(this->timer_fd.fd(), 0, &timeout, nullptr) < 0) {
        LOG(LOG_ERR, "unable to set the timer time");
        throw Error(ERR_TRANSPORT_READ_FAILED, errno);
    }
}

void ReplayTransport::read_more_chunk()
{
    while (!this->is_eof) {
        auto header = read_recorder_transport_header(this->in_file);

        this->record_time = this->start_time + header.record_duration;

        switch (header.type)
        {
            case RecorderTransport::PacketType::ClientCert:
                this->public_key.size = header.data_size;
                this->public_key.data = std::make_unique<uint8_t[]>(header.data_size);
                this->in_file.recv_boom(this->public_key.data.get(), this->public_key.size);
                break;

            case RecorderTransport::PacketType::DataIn:
                if (this->data.capacity < header.data_size) {
                    this->data.data = std::make_unique<uint8_t[]>(header.data_size);
                    this->data.capacity = header.data_size;
                }
                this->data.size = header.data_size;
                this->data.current_pos = 0;
                this->in_file.recv_boom(this->data.data.get(), header.data_size);
                // this->record_len += header.data_size;
                if (header.data_size) {
                    return;
                }
                break;

            case RecorderTransport::PacketType::Eof:
                LOG(LOG_INFO, "ReplayTransport::read_more_chunk: eof = true");
                this->is_eof = true;
                return;

            case RecorderTransport::PacketType::DataOut:
            case RecorderTransport::PacketType::Connect:
            case RecorderTransport::PacketType::Disconnect:
            case RecorderTransport::PacketType::ServerCert: {
                // TODO
                uint8_t buffer[4*1024];
                while (header.data_size) {
                    auto min = std::min<size_t>(sizeof(buffer), header.data_size);
                    this->in_file.recv_boom(buffer, min);
                    header.data_size -= min;
                }
                break;
            }
        }
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

    return Transport::TlsResult::Ok;
}

array_view_const_u8 ReplayTransport::Data::av() const noexcept
{
    return {this->data.get() + this->current_pos, this->size - this->current_pos};
}

size_t ReplayTransport::do_partial_read(uint8_t * buffer, size_t const len)
{
    if (this->is_eof) {
        LOG(LOG_INFO, "ReplayTransport::do_partial_read: is eof");
        throw Error(ERR_TRANSPORT_NO_MORE_DATA);
    }

    if (Timing::Real == this->timing)
    {
        uint64_t timeval;
        if (sizeof(timeval) != read(this->timer_fd.fd(), &timeval, sizeof(timeval))) {
            int const err = errno;
            LOG(LOG_ERR, "ReplayTransport::do_partial_read: read timer_fd error");
            throw Error(ERR_TRANSPORT_NO_MORE_DATA, err);
        }
    }

    auto av = this->data.av();

    if (av.size() > len) {
        LOG(LOG_ERR, "ReplayTransport::partial_read(buf, len=%zu) should be %zu or greater", len, av.size());
        throw Error(ERR_TRANSPORT_DIFFERS);
    }

    auto new_len = std::min(av.size(), len);
    memcpy(buffer, av.data(), new_len);
    this->data.current_pos += new_len;

    LOG(LOG_DEBUG, "ReplayTransport::partial_read len=%zu new_len=%zu remaining=%zu", len, new_len, av.size());

    if (av.size() == new_len) {
        this->read_more_chunk();
    }
    this->reschedule_timer();

    return new_len;
}

Transport::Read ReplayTransport::do_atomic_read(uint8_t * buffer, size_t len)
{
    (void)buffer;
    (void)len;
    LOG(LOG_ERR, "ReplayTransport::do_atomic_read unimplemented");
    throw Error(ERR_TRANSPORT_READ_FAILED);
}

void ReplayTransport::do_send(const uint8_t * const buffer, size_t len)
{
    (void)buffer;
    (void)len;
}
