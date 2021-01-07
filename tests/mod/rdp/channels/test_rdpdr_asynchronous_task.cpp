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


#include "test_only/test_framework/redemption_unit_tests.hpp"
#include "test_only/test_framework/file.hpp"


#include "utils/sugar/unique_fd.hpp"
#include "gdi/graphic_api.hpp"
#include "mod/rdp/channels/rdpdr_asynchronous_task.hpp"
#include "test_only/transport/test_transport.hpp"
#include "core/events.hpp"

#include <deque>


class TestToServerSender : public VirtualChannelDataSender
{
    Transport & transport;

public:
    TestToServerSender(Transport & transport) : transport(transport) {}

    void operator() (uint32_t total_length, uint32_t flags, bytes_view chunk_data) override {
        StaticOutStream<8> stream;
        stream.out_uint32_le(total_length);
        stream.out_uint32_le(flags);

        this->transport.send(stream.get_produced_bytes());
        this->transport.send(chunk_data);
    }
};

RED_AUTO_TEST_CASE(TestRdpdrDriveReadTask)
{
    int fd = ::open(FIXTURES_PATH "/rfc959.txt", O_RDONLY);
    RED_CHECK_NE(fd, -1);
    unique_fd ufd{fd};

    //LogTransport log_transport;
    //TestToServerSender test_to_server_sender(log_transport);

    #include "fixtures/test_rdpdr_drive_read_task.hpp"
    CheckTransport check_transport(cstr_array_view(outdata));
    TestToServerSender test_to_server_sender(check_transport);

    const uint32_t DeviceId = 0;
    const uint32_t CompletionId = 8;

    const uint32_t number_of_bytes_to_read = 2 * 1024;

    EventContainer events;

    AsynchronousTaskContainer tasks(events);
    tasks.add(std::make_unique<RdpdrDriveReadTask>(
        fd, DeviceId, CompletionId, number_of_bytes_to_read, 1024 * 32,
        test_to_server_sender,
        RDPVerbose(0)));

    RED_CHECK(!events.queue.empty());
    timeval now = events.get_current_time();
    for (int i = 0; i < 100 && !events.queue.empty(); ++i) {
        events.set_current_time(now);
        events.execute_events([](int/*fd*/){ return true; }, false);
        ++now.tv_sec;
    }
    RED_CHECK(events.queue.empty());
}

RED_AUTO_TEST_CASE(TestRdpdrSendDriveIOResponseTask)
{
    auto contents = RED_REQUIRE_GET_FILE_CONTENTS(FIXTURES_PATH "/sample.bmp");
    RED_CHECK_EQUAL(1974u, contents.size());

    //LogTransport log_transport;
    //TestToServerSender test_to_server_sender(log_transport);

    #include "fixtures/test_rdpdr_send_drive_io_response_task.hpp"
    CheckTransport check_transport(cstr_array_view(outdata));

    TestToServerSender test_to_server_sender(check_transport);

    EventContainer events;

    AsynchronousTaskContainer tasks(events);
    tasks.add(std::make_unique<RdpdrSendDriveIOResponseTask>(
        CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
        byte_ptr_cast(contents.data()),
        contents.size(), test_to_server_sender,
        RDPVerbose(0)));

    RED_CHECK(!events.queue.empty());

    timeval now = events.get_current_time();
    for (int i = 0; i < 100 && !events.queue.empty(); ++i) {
        events.set_current_time(now);
        events.execute_events([](int/*fd*/){return false;}, false);
        ++now.tv_sec;
    }
    RED_CHECK(events.queue.empty());
}

RED_AUTO_TEST_CASE(TestAsynchronousRemoved)
{
    CheckTransport check_transport(""_av);

    TestToServerSender test_to_server_sender(check_transport);

    EventContainer events;

    AsynchronousTaskContainer tasks(events);
    tasks.add(std::make_unique<RdpdrSendDriveIOResponseTask>(
        CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
        byte_ptr_cast("abc"), 3, test_to_server_sender,
        RDPVerbose(0)));

    RED_CHECK(!events.queue.empty());

    // event is removed
}
