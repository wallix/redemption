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


#define RED_TEST_MODULE TestRDPDRAsynchronousTask
#include "system/redemption_unit_tests.hpp"


#include "test_only/get_file_contents.hpp"
#include "transport/in_file_transport.hpp"
#include "utils/log.hpp"
#include "utils/sugar/make_unique.hpp"
#include "mod/rdp/channels/rdpdr_asynchronous_task.hpp"
#include "test_only/transport/test_transport.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


class TestToServerSender : public VirtualChannelDataSender {
    Transport & transport;

public:
    TestToServerSender(Transport & transport) : transport(transport) {}

    virtual void operator() (uint32_t total_length, uint32_t flags,
        const uint8_t * chunk_data, uint32_t chunk_data_length) override {
        LOG(LOG_INFO, "total_length=%u flags=0x%X chunk_data=<%p> chunk_data_length=%u",
            total_length, flags, static_cast<void const *>(chunk_data), chunk_data_length);
        StaticOutStream<8> stream;
        stream.out_uint32_le(total_length);
        stream.out_uint32_le(flags);

        this->transport.send(stream.get_data(), stream.get_offset());
        this->transport.send(chunk_data, chunk_data_length);
    }
};

RED_AUTO_TEST_CASE(TestRdpdrDriveReadTask)
{
    uint32_t verbose = 1;

    int fd = ::open(FIXTURES_PATH "/rfc959.txt", O_RDONLY);
    RED_CHECK_NE(fd, -1);
    unique_fd ufd{fd};

    //LogTransport log_transport;
    //TestToServerSender test_to_server_sender(log_transport);

    #include "fixtures/test_rdpdr_drive_read_task.hpp"
    CheckTransport check_transport(outdata, sizeof(outdata)-1);
    TestToServerSender test_to_server_sender(check_transport);

    const uint32_t DeviceId = 0;
    const uint32_t CompletionId = 8;

    const uint32_t number_of_bytes_to_read = 2 * 1024;

    RdpdrDriveReadTask rdpdr_drive_read_task(
        fd, DeviceId, CompletionId, number_of_bytes_to_read, 1024 * 32,
        test_to_server_sender, to_verbose_flags(verbose));

    bool run_task = true;

    do
    {
        wait_obj event;

        SessionReactor session_reactor;
        rdpdr_drive_read_task.configure_event(
            session_reactor, {&run_task, [](bool* b, AsynchronousTask&) noexcept {
                *b = false;
            }});

        unsigned max = 0;
        fd_set rfds;

        FD_ZERO(&rfds);

        timeval timeout = { 3, 0 };

        event.wait_on_fd(fd, rfds, max, timeout);

        int num = select(max + 1, &rfds, nullptr, nullptr, &timeout);

        if (num < 0) {
            if (errno == EINTR) {
                continue;
            }

            LOG(LOG_ERR, "Task loop raised error %d : %s", errno, strerror(errno));
            run_task = false;
        }
        else {
            if (event.is_set(fd, rfds)) {
                auto& c = session_reactor.fd_events_.elements;
                if (!c[0]->exec()) {
                    c.erase(c.begin());
                }
            }
        }
    }
    while (run_task);
}

RED_AUTO_TEST_CASE(TestRdpdrSendDriveIOResponseTask)
{
    uint32_t verbose = 1;

    auto contents = get_file_contents<std::string>(FIXTURES_PATH "/sample.bmp");
    RED_CHECK_EQUAL(1974u, contents.size());

    //LogTransport log_transport;
    //TestToServerSender test_to_server_sender(log_transport);

    #include "fixtures/test_rdpdr_send_drive_io_response_task.hpp"
    LOG(LOG_INFO, "CHECK_EXCEPTION_ERROR_ID");
    CheckTransport check_transport(outdata, sizeof(outdata)-1);

    LOG(LOG_INFO, "check_transport");
    TestToServerSender test_to_server_sender(check_transport);

    LOG(LOG_INFO, "TestToServerSender");

    RdpdrSendDriveIOResponseTask rdpdr_send_drive_io_response_task(
        CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
        reinterpret_cast<uint8_t const *>(contents.data()),
        contents.size(),
        test_to_server_sender,
        to_verbose_flags(verbose));

    LOG(LOG_INFO, "RdpdrSendDriveIOResponseTask");

    bool run_task = true;

    do
    {
        LOG(LOG_INFO, "do");
        wait_obj event;

        SessionReactor session_reactor;
        rdpdr_send_drive_io_response_task.configure_event(
            session_reactor, {&run_task, [](bool* b, AsynchronousTask&) noexcept {
                *b = false;
            }});

        unsigned max = 0;
        fd_set rfds;

        FD_ZERO(&rfds);

        timeval timeout = { 3, 0 };

        event.wait_on_fd(INVALID_SOCKET, rfds, max, timeout);

        int num = select(max + 1, &rfds, nullptr, nullptr, &timeout);

        if (num < 0) {
            if (errno == EINTR) {
                continue;
            }

            LOG(LOG_ERR, "Task loop raised error %d : %s", errno, strerror(errno));
            run_task = false;
        }
        else {
            if (event.is_set(INVALID_SOCKET, rfds)) {
                session_reactor.timer_events_.exec(timeout);
            }
        }
    }
    while (run_task);
}
