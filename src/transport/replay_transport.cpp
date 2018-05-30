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
    const char* fname, const char *ip_address, int port, FdType fd_type)
: start_time(std::chrono::system_clock::now())
, in_file(open_file(fname))
, fd(FdType::Timer == fd_type
? timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK)
: []{
    int fd = eventfd(1, EFD_NONBLOCK);
    if (fd > -1) {
        uint64_t value = 0;
        write(fd, &value, 8); // that will make the file descriptor always selectable
    }
    return fd;
}())
, fd_type(fd_type)
{
    (void)ip_address;
    (void)port;

    if (!this->fd) {
        throw Error(ERR_TRANSPORT_OPEN_FAILED, errno);
    }

    this->read_more_chunk();
}


ReplayTransport::~ReplayTransport() = default;


void ReplayTransport::reschedule_timer()
{
    if (FdType::Timer != this->fd_type) {
        return;
    }

    auto const now = std::chrono::system_clock::now();
    // zero disarms the timer, force to 1 nanoseconds
    auto const delate_time = std::max(
        std::chrono::duration_cast<std::chrono::nanoseconds>(this->record_time - now),
        std::chrono::nanoseconds{1});
    auto const sec = std::chrono::duration_cast<std::chrono::seconds>(delate_time);
    auto const nano = delate_time - sec;

    itimerspec timeout = {};
    timeout.it_value.tv_sec = sec.count();
    timeout.it_value.tv_nsec = nano.count();

    // LOG(LOG_DEBUG, "trigerring timer now sec=%ld nsec=%ld", timeout.it_value.tv_sec, timeout.it_value.tv_nsec);

    if (timerfd_settime(this->fd.fd(), 0, &timeout, nullptr) < 0) {
        LOG(LOG_ERR, "unable to set the timer time");
        throw Error(ERR_TRANSPORT_READ_FAILED, errno);
    }
}

using PacketType = RecorderTransport::PacketType;

void ReplayTransport::read_more_chunk()
{
    this->data_pos = 0;
    while (!this->datas.empty()) {
        this->gc_datas.push_back(std::move(this->datas.back()));
        this->datas.pop_back();
    }

    while (!this->is_eof) {
        auto read_buffer = [this](PacketType type, size_t const n){
            Data* buf;
            if (this->gc_datas.empty()) {
                buf = &this->datas.emplace_back();
                auto capacity = std::max(n, size_t(1024u));
                buf->data = std::make_unique<uint8_t[]>(capacity);
                buf->capacity = capacity;
            }
            else {
                buf = &this->datas.emplace_back(std::move(this->gc_datas.back()));
                this->gc_datas.pop_back();
                if (buf->capacity < n) {
                    buf->capacity = n + 1024u;
                    buf->data = std::make_unique<uint8_t[]>(buf->capacity);
                }
            };

            buf->size = n;
            buf->type = type;

            this->in_file.recv_boom(buf->data.get(), n);
        };

        auto header = read_recorder_transport_header(this->in_file);

        this->record_time = this->start_time + header.record_duration;

        switch (header.type)
        {
            case PacketType::ClientCert:
            case PacketType::DataOut:
            case PacketType::ServerCert:
            case PacketType::Connect:
            case PacketType::Disconnect:
                read_buffer(header.type, header.data_size);
                break;

            case PacketType::DataIn:
            case PacketType::Eof:
                read_buffer(header.type, header.data_size);
                this->reschedule_timer();
                return;
        }
    }
}

array_view_const_u8 ReplayTransport::next_current_data(PacketType type)
{
    auto const no_more = (this->datas.size() < this->data_pos);
    if (no_more || this->datas[this->data_pos].type != type) {
        if (no_more) {
            LOG(LOG_ERR, "ReplayTransport: no data");
        }
        else {
            LOG(LOG_ERR, "ReplayTransport: next type is %d, expected %d",
                this->datas[this->data_pos].type, type);
        }
        throw Error(no_more || this->is_eof ? ERR_TRANSPORT_NO_MORE_DATA : ERR_TRANSPORT_DIFFERS);
    }

    return this->datas[this->data_pos++].av();
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

    auto av = this->next_current_data(PacketType::ClientCert);
    this->public_key.size = av.size();
    this->public_key.data = std::make_unique<uint8_t[]>(av.size());
    memcpy(this->public_key.data.get(), av.data(), av.size());
    return Transport::TlsResult::Ok;
}

void ReplayTransport::enable_server_tls(
    const char* certificate_password, const char* ssl_cipher_list)
{
    this->next_current_data(PacketType::ServerCert);
    (void)certificate_password;
    (void)ssl_cipher_list;
}

bool ReplayTransport::connect()
{
    this->next_current_data(PacketType::Connect);
    return true;
}

bool ReplayTransport::disconnect()
{
    this->next_current_data(PacketType::Disconnect);
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

size_t ReplayTransport::do_partial_read(uint8_t * buffer, size_t const len)
{
    if (this->is_eof) {
        throw Error(ERR_TRANSPORT_NO_MORE_DATA);
    }

    this->read_timer();

    auto av = this->next_current_data(PacketType::DataIn);

    if (av.size() > len) {
        LOG(LOG_ERR, "ReplayTransport::do_atomic_read(buf, len=%zu) should be %zu or greater", len, av.size());
        throw Error(ERR_TRANSPORT_DIFFERS);
    }

    memcpy(buffer, av.data(), av.size());
    this->read_more_chunk();
    return av.size();
}

Transport::Read ReplayTransport::do_atomic_read(uint8_t * buffer, size_t len)
{
    if (this->is_eof) {
        return Read::Eof;
    }

    this->read_timer();

    auto av = this->next_current_data(PacketType::DataIn);

    if (av.size() != len) {
        LOG(LOG_ERR, "ReplayTransport::do_atomic_read(buf, len=%zu) should be %zu", len, av.size());
        throw Error(ERR_TRANSPORT_DIFFERS);
    }

    memcpy(buffer, av.data(), len);
    this->read_more_chunk();
    return Read::Ok;
}

void ReplayTransport::do_send(const uint8_t * const buffer, size_t len)
{
    auto av = this->next_current_data(PacketType::DataOut);

    if (av.size() != len || memcmp(av.data(), buffer, len)) {
        if (av.size() != len) {
            LOG(LOG_ERR, "ReplayTransport::do_send(buf, len=%zu) should be %zu", len, av.size());
        }
        else {
            LOG(LOG_ERR, "ReplayTransport::do_send data differs");
        }
        throw Error(ERR_TRANSPORT_DIFFERS);
    }
}
