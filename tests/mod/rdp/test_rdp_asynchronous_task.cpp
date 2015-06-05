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
#define BOOST_TEST_MODULE TestRDPAsynchronousTask
#include <boost/test/auto_unit_test.hpp>

#define LOGNULL
//#define LOGPRINT

#include "in_file_transport.hpp"
#include "log.hpp"
#include "make_unique.hpp"
#include "rdp/rdp_asynchronous_task.hpp"
#include "test_transport.hpp"

class TestToServerSender : public ToServerSender {
    Transport & transport;

public:
    TestToServerSender(Transport & transport) : transport(transport) {}

    virtual void operator() (size_t total_length, uint32_t flags,
        const uint8_t * chunk_data, size_t chunk_data_length) override {
        BStream stream(128);
        stream.out_uint32_le(total_length);
        stream.out_uint32_le(flags);
        stream.mark_end();

        this->transport.send(stream);
        this->transport.send(chunk_data, chunk_data_length);
    }
};

class FdWrapper {
public:
    int fd = -1;

    explicit FdWrapper(int fd) : fd(fd) {}

    ~FdWrapper() {
        if (this->fd != -1) {
            ::close(this->fd);
        }
    }
};

BOOST_AUTO_TEST_CASE(TestRdpdrDriveReadTask)
{
    uint32_t verbose = 1;

    FdWrapper fd_wrapper(::open("tests/fixtures/rfc959.txt", O_RDONLY));
    if (fd_wrapper.fd == -1) { throw Error(ERR_TRANSPORT_OPEN_FAILED); }

    auto transport = std::make_unique<InFileTransport>(fd_wrapper.fd);

    //CheckTransport check_transport("0123456789", 10, verbose);
    LogTransport log_transport;

//    TestToServerSender test_to_server_sender(check_transport);
    TestToServerSender test_to_server_sender(log_transport);

    const uint32_t DeviceId = 0;
    const uint32_t CompletionId = 8;

    const uint32_t number_of_bytes_to_read = 2 * 1024;

    RdpdrDriveReadTask rdpdr_drive_read_task(std::move(transport), test_to_server_sender,
        DeviceId, CompletionId, number_of_bytes_to_read, verbose);

    while (rdpdr_drive_read_task.run());
}
