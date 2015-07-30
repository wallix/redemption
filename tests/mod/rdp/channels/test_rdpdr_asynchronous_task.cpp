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


#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestRDPDRAsynchronousTask
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
//#define LOGPRINT

#include "fdbuf.hpp"
#include "in_file_transport.hpp"
#include "log.hpp"
#include "make_unique.hpp"
#include "rdp/channels/rdpdr_asynchronous_task.hpp"
#include "socket_transport_utility.hpp"
#include "test_transport.hpp"

class TestToServerSender : public VirtualChannelDataSender {
    Transport & transport;

public:
    TestToServerSender(Transport & transport) : transport(transport) {}

    virtual void operator() (uint32_t total_length, uint32_t flags,
        const uint8_t * chunk_data, uint32_t chunk_data_length) override {
        LOG(LOG_INFO, "total_length=%u flags=0x%X chunk_data=<%p> chunk_data_length=%u",
            static_cast<uint32_t>(total_length), flags,
            chunk_data, static_cast<uint32_t>(chunk_data_length));
        BStream stream(128);
        stream.out_uint32_le(total_length);
        stream.out_uint32_le(flags);
        stream.mark_end();

        this->transport.send(stream);
        this->transport.send(chunk_data, chunk_data_length);
    }
};

BOOST_AUTO_TEST_CASE(TestRdpdrDriveReadTask)
{
    uint32_t verbose = 1;

    int fd = ::open("tests/fixtures/rfc959.txt", O_RDONLY);
    if (fd == -1) { throw Error(ERR_TRANSPORT_OPEN_FAILED); }

    io::posix::fdbuf fd_wrapper(fd);

    std::unique_ptr<InFileTransport> transport = std::make_unique<InFileTransport>(fd);

    fd_wrapper.release();

    //LogTransport log_transport;
    //TestToServerSender test_to_server_sender(log_transport);

    #include "fixtures/test_rdpdr_drive_read_task.hpp"
    CheckTransport check_transport(outdata, sizeof(outdata), verbose);
    TestToServerSender test_to_server_sender(check_transport);

    const uint32_t DeviceId = 0;
    const uint32_t CompletionId = 8;

    const uint32_t number_of_bytes_to_read = 2 * 1024;

    RdpdrDriveReadTask rdpdr_drive_read_task(transport.get(), fd,
        DeviceId, CompletionId, number_of_bytes_to_read, 1024 * 32,
        test_to_server_sender, verbose);

    bool run_task = true;

    do
    {
        wait_obj event;

        rdpdr_drive_read_task.configure_wait_object(event);

        unsigned max = 0;
        fd_set rfds;

        FD_ZERO(&rfds);

        timeval timeout = { 3, 0 };

        add_to_fd_set(event, rdpdr_drive_read_task.get_file_descriptor(), rfds, max, timeout);

        int num = select(max + 1, &rfds, nullptr, nullptr, &timeout);

        if (num < 0) {
            if (errno == EINTR) {
                continue;
            }

            LOG(LOG_ERR, "Task loop raised error %u : %s", errno, strerror(errno));
            run_task = false;
        }
        else {
            if (is_set(event, rdpdr_drive_read_task.get_file_descriptor(), rfds)) {
                if (!rdpdr_drive_read_task.run(event)) {
                    run_task = false;
                }
            }
        }
    }
    while (run_task);
}

BOOST_AUTO_TEST_CASE(TestRdpdrSendDriveIOResponseTask)
{
    uint32_t verbose = 1;

    int fd = ::open("tests/fixtures/sample.bmp", O_RDONLY);
    if (fd == -1) { throw Error(ERR_TRANSPORT_OPEN_FAILED); }

    io::posix::fdbuf fd_wrapper(fd);

    std::unique_ptr<InFileTransport> transport = std::make_unique<InFileTransport>(fd);

    fd_wrapper.release();

    BStream stream(65536);

    try {
        transport->recv(&stream.p, CHANNELS::CHANNEL_CHUNK_LENGTH + 1024);
    }
    catch (Error & e) {
        if(e.id != 1501) {
            LOG(LOG_ERR, "Error = %d", e.id);

            throw;
        }
    }

    stream.mark_end();

    //LogTransport log_transport;
    //TestToServerSender test_to_server_sender(log_transport);

    #include "fixtures/test_rdpdr_send_drive_io_response_task.hpp"
    CheckTransport check_transport(outdata, sizeof(outdata), verbose);
    TestToServerSender test_to_server_sender(check_transport);

    RdpdrSendDriveIOResponseTask rdpdr_send_drive_io_response_task(
        CHANNELS::CHANNEL_FLAG_FIRST | CHANNELS::CHANNEL_FLAG_LAST,
        stream.get_data(),
        stream.size(),
        test_to_server_sender,
        verbose);

    bool run_task = true;

    do
    {
        wait_obj event;

        rdpdr_send_drive_io_response_task.configure_wait_object(event);

        unsigned max = 0;
        fd_set rfds;

        FD_ZERO(&rfds);

        timeval timeout = { 3, 0 };

        add_to_fd_set(event, rdpdr_send_drive_io_response_task.get_file_descriptor(), rfds, max, timeout);

        int num = select(max + 1, &rfds, nullptr, nullptr, &timeout);

        if (num < 0) {
            if (errno == EINTR) {
                continue;
            }

            LOG(LOG_ERR, "Task loop raised error %u : %s", errno, strerror(errno));
            run_task = false;
        }
        else {
            if (is_set(event, rdpdr_send_drive_io_response_task.get_file_descriptor(), rfds)) {
                if (!rdpdr_send_drive_io_response_task.run(event)) {
                    run_task = false;
                }
            }
        }
    }
    while (run_task);
}
