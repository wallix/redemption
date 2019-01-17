/*
    This program is free software; you can redistribute it and/or modify it
     under the terms of the GNU General Public License as published by the
     Free Software Foundation; either version 2 of the License, or (at your
     option) any later version.

    This program is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
     Public License for more details.

    You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
     675 Mass Ave, Cambridge, MA 02139, USA.

    Product name: redemption, a FLOSS RDP proxy
    Copyright (C) Wallix 2015
    Author(s): Christophe Grosjean, Raphael Zhou
*/


#pragma once

#include "utils/asynchronous_task_manager.hpp"
#include "core/channel_list.hpp"
#include "mod/rdp/channels/virtual_channel_data_sender.hpp"
#include "core/RDP/channels/rdpdr.hpp"
#include "core/session_reactor.hpp"
#include "mod/rdp/rdp_verbose.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>

inline void rdpdr_in_file_seek(int fd, int64_t offset, int whence)
{
    if (static_cast<off64_t>(-1) == lseek64(fd, offset, whence)){
        throw Error(ERR_TRANSPORT_SEEK_FAILED, errno);
    }
}

inline size_t rdpdr_in_file_read(int fd, uint8_t * buffer, size_t len)
{
    size_t remaining_len = len;
    while (remaining_len) {
        ssize_t res = ::read(fd, buffer + (len - remaining_len), remaining_len);
        if (res <= 0){
            if (errno == EINTR){
                continue;
            }
            if (res == 0 || remaining_len != len) {
                break;
            }
            throw Error(ERR_TRANSPORT_READ_FAILED, res);
        }
        remaining_len -= res;
    }
    return len - remaining_len;
}

namespace detail
{
    inline auto create_notify_delete_task() noexcept
    {
        return [](
            jln::NotifyDeleteType d,
            AsynchronousTask& task,
            AsynchronousTask::TerminateEventNotifier& terminate_notifier
        ){
            if (d == jln::NotifyDeleteType::DeleteByAction) {
                terminate_notifier(task);
            }
        };
    }
}  // namespace detail

class RdpdrDriveReadTask final : public AsynchronousTask
{
    const int file_descriptor;

    const uint32_t DeviceId;
    const uint32_t CompletionId;

    const uint32_t total_number_of_bytes_to_read;
          uint32_t remaining_number_of_bytes_to_read = 0;

    uint32_t length = 0;

    off64_t Offset;

    VirtualChannelDataSender & to_server_sender;

    SessionReactor::TopFdPtr fdobject;

    const RDPVerbose verbose;

public:
    RdpdrDriveReadTask(int file_descriptor,
                       uint32_t DeviceId,
                       uint32_t CompletionId,
                       uint32_t number_of_bytes_to_read,
                       off64_t Offset,
                       VirtualChannelDataSender & to_server_sender,
                       RDPVerbose verbose)
    : file_descriptor(file_descriptor)
    , DeviceId(DeviceId)
    , CompletionId(CompletionId)
    , total_number_of_bytes_to_read(number_of_bytes_to_read)
    , remaining_number_of_bytes_to_read(number_of_bytes_to_read)
    , Offset(Offset)
    , to_server_sender(to_server_sender.SynchronousSender())
    , verbose(verbose)
    {}

    void configure_event(SessionReactor& session_reactor, TerminateEventNotifier terminate_notifier) override
    {
        assert(!this->fdobject);
        this->fdobject = session_reactor.create_fd_event(
            this->file_descriptor, std::ref(*this), terminate_notifier)
        .on_action([](auto ctx, RdpdrDriveReadTask& self, TerminateEventNotifier& terminate_notifier) {
            if (self.run()) {
                return ctx.need_more_data();
            }
            auto const r = ctx.terminate();
            self.fdobject.detach();
            terminate_notifier(self); // destroy this
            return r;
        })
        .on_exit(jln::propagate_exit())
        .set_timeout(std::chrono::seconds(1))
        .on_timeout([](auto ctx, RdpdrDriveReadTask& self, TerminateEventNotifier&){
            LOG(LOG_WARNING, "RdpdrDriveReadTask::run: File (%d) is not ready!",
                self.file_descriptor);
            return ctx.ready();
        });
    }

    bool run()
    {
        StaticOutStream<CHANNELS::CHANNEL_CHUNK_LENGTH> out_stream;

        uint32_t out_flags = 0;

        if (this->remaining_number_of_bytes_to_read == this->total_number_of_bytes_to_read) {
            const rdpdr::SharedHeader sh_s(rdpdr::Component::RDPDR_CTYP_CORE,
                                           rdpdr::PacketId::PAKID_CORE_DEVICE_IOCOMPLETION);
            sh_s.emit(out_stream);

            const rdpdr::DeviceIOResponse device_io_response(
                    DeviceId,
                    CompletionId,
                    erref::NTSTATUS::STATUS_SUCCESS
                );
            if (bool(this->verbose & RDPVerbose::asynchronous_task)) {
                LOG(LOG_INFO, "RdpdrDriveReadTask::run");
                device_io_response.log(LOG_INFO);
            }
            device_io_response.emit(out_stream);

            out_stream.out_uint32_le(this->total_number_of_bytes_to_read);  // length(4)

            if (bool(this->verbose & RDPVerbose::asynchronous_task)) {
                LOG(LOG_INFO, "RdpdrDriveReadTask::run: Length=%u",
                    this->remaining_number_of_bytes_to_read);
            }

            out_flags |= CHANNELS::CHANNEL_FLAG_FIRST;

            assert(!this->length);

            this->length = out_stream.get_offset() + this->total_number_of_bytes_to_read;
        }

        const uint32_t number_of_bytes_to_read =
            std::min<uint32_t>(out_stream.tailroom(), this->remaining_number_of_bytes_to_read);

        if (bool(this->verbose & RDPVerbose::asynchronous_task)) {
            LOG(LOG_INFO, "RdpdrDriveReadTask::run: NumberOfBytesToRead=%u",
                number_of_bytes_to_read);
        }

        rdpdr_in_file_seek(this->file_descriptor, this->Offset, SEEK_SET);

        try {
            auto number_of_bytes_read = rdpdr_in_file_read(this->file_descriptor, out_stream.get_current(), number_of_bytes_to_read);
            out_stream.out_skip_bytes(number_of_bytes_read);
            if (bool(this->verbose & RDPVerbose::asynchronous_task)) {
                LOG(LOG_INFO, "RdpdrDriveReadTask::run: NumberOfBytesRead=%zu", number_of_bytes_read);
            }
            this->Offset += number_of_bytes_read;
            this->remaining_number_of_bytes_to_read -= number_of_bytes_read;
            if (0 == this->remaining_number_of_bytes_to_read) {
                out_flags |= CHANNELS::CHANNEL_FLAG_LAST;
            }

            assert(this->length);

            this->to_server_sender(this->length, out_flags, out_stream.get_data(), out_stream.get_offset());
        }
        catch (const Error & e) {
            LOG(LOG_INFO, "RdpdrDriveReadTask::run: Exception=%u", e.id);
            throw;
        }
        return (this->remaining_number_of_bytes_to_read != 0);
    }
};  // RdpdrDriveReadTask

class RdpdrSendDriveIOResponseTask final : public AsynchronousTask
{
    const uint32_t flags;
    std::unique_ptr<uint8_t[]> data;
    const size_t data_length;

    size_t remaining_number_of_bytes_to_send;

    VirtualChannelDataSender & to_server_sender;

    SessionReactor::TimerPtr timer_ptr;

public:
    RdpdrSendDriveIOResponseTask(uint32_t flags,
                                 const uint8_t * data,
                                 size_t data_length,
                                 VirtualChannelDataSender & to_server_sender,
                                 RDPVerbose verbose)
    : flags(flags)
    , data(std::make_unique<uint8_t[]>(data_length))
    , data_length(data_length)
    , remaining_number_of_bytes_to_send(data_length)
    , to_server_sender(to_server_sender.SynchronousSender()) {
        (void)verbose;
        ::memcpy(this->data.get(), data, data_length);
    }

    void configure_event(SessionReactor& session_reactor, TerminateEventNotifier terminate_notifier) override
    {
        assert(!this->timer_ptr);
        // TODO create_yield_event
        this->timer_ptr = session_reactor.create_timer(
            std::ref(*this), terminate_notifier)
        .set_notify_delete(detail::create_notify_delete_task())
        .set_delay(std::chrono::milliseconds(1))
        .on_action([](auto ctx, RdpdrSendDriveIOResponseTask& self, TerminateEventNotifier&){
            return self.run() ? ctx.ready() : ctx.terminate();
        });
    }

    bool run()
    {
        if (this->data_length <= CHANNELS::CHANNEL_CHUNK_LENGTH) {
            this->to_server_sender(this->data_length, this->flags, this->data.get(), this->data_length);

            this->remaining_number_of_bytes_to_send = 0;

            return false;
        }

        assert(this->remaining_number_of_bytes_to_send);

        uint32_t out_flags = this->flags;

        if (this->remaining_number_of_bytes_to_send != this->data_length) {
            out_flags &= ~CHANNELS::CHANNEL_FLAG_FIRST;
        }

        uint32_t number_of_bytes_sent = this->data_length - this->remaining_number_of_bytes_to_send;
        uint32_t number_of_bytes_to_send =
            std::min<uint32_t>(this->remaining_number_of_bytes_to_send, CHANNELS::CHANNEL_CHUNK_LENGTH);

        if (number_of_bytes_to_send != this->remaining_number_of_bytes_to_send) {
            out_flags &= ~CHANNELS::CHANNEL_FLAG_LAST;
        }

        this->to_server_sender(this->data_length, out_flags,
            this->data.get() + number_of_bytes_sent,
            number_of_bytes_to_send);

        this->remaining_number_of_bytes_to_send -= number_of_bytes_to_send;

        return (this->remaining_number_of_bytes_to_send != 0);
    }
};

class RdpdrSendClientMessageTask final : public AsynchronousTask {
    const size_t total_length;
    const uint32_t flags;
    std::unique_ptr<uint8_t[]> chunked_data;
    const size_t chunked_data_length;

    VirtualChannelDataSender & to_server_sender;

    SessionReactor::TimerPtr timer_ptr;

public:
    RdpdrSendClientMessageTask(
        size_t total_length,
        uint32_t flags,
        const uint8_t * chunked_data,
        size_t chunked_data_length,
        VirtualChannelDataSender & to_server_sender,
        RDPVerbose verbose)
    : total_length(total_length)
    , flags(flags)
    , chunked_data(std::make_unique<uint8_t[]>(chunked_data_length))
    , chunked_data_length(chunked_data_length)
    , to_server_sender(to_server_sender.SynchronousSender())
    {
        assert(chunked_data_length <= CHANNELS::CHANNEL_CHUNK_LENGTH);
        (void)verbose;

        ::memcpy(this->chunked_data.get(), chunked_data, chunked_data_length);
    }

    void configure_event(SessionReactor& session_reactor, TerminateEventNotifier terminate_notifier) override
    {
        assert(!this->timer_ptr);
        // TODO create_yield_event
        this->timer_ptr = session_reactor.create_timer(
            std::ref(*this), terminate_notifier)
        .set_notify_delete(detail::create_notify_delete_task())
        .set_delay(std::chrono::milliseconds(1))
        .on_action([](auto ctx, RdpdrSendClientMessageTask& self, TerminateEventNotifier&){
            self.run();
            return ctx.terminate();
        });
    }

    bool run()
    {
        assert(this->chunked_data_length <=
            CHANNELS::CHANNEL_CHUNK_LENGTH);

        this->to_server_sender(this->total_length, this->flags,
            this->chunked_data.get(), this->chunked_data_length);

        return false;
    }
};
