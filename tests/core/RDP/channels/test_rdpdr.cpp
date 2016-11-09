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
    Copyright (C) Wallix 2016
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestGCC
#include "system/redemption_unit_tests.hpp"

#define LOGNULL
//#define LOGPRINT
#include <iostream>
#include "core/RDP/channels/rdpdr.hpp"

BOOST_AUTO_TEST_CASE(TestDeviceCreateRequest1)
{
    const char in_data[] =
            "\x81\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
            "\x07\x00\x00\x00\x01\x00\x00\x00\x20\x00\x00\x00\x0e\x00\x00\x00" // ........ .......
            "\x5c\x00\x2e\x00\x63\x00\x70\x00\x61\x00\x6e\x00\x00\x00"         // ....c.p.a.n...
        ;
    InStream in_stream(in_data, sizeof(in_data) - 1);

    rdpdr::DeviceCreateRequest device_create_request;

    device_create_request.receive(in_stream);

    //device_create_request.log(LOG_INFO);

    char out_data[sizeof(in_data)];

    OutStream out_stream(out_data, sizeof(out_data));

    device_create_request.emit(out_stream);
    //LOG(LOG_INFO, "out_stream_size=%u", (unsigned)out_stream.get_offset());
    //hexdump(out_stream.get_data(), out_stream.get_offset());

    BOOST_CHECK_EQUAL(out_stream.get_offset(), in_stream.get_offset());
    BOOST_CHECK_EQUAL(0, memcmp(in_data, out_data, sizeof(in_data) - 1));
}

BOOST_AUTO_TEST_CASE(TestDeviceCreateRequest2)
{
    const char in_data[] =
            "\x89\x00\x12\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
            "\x07\x00\x00\x00\x01\x00\x00\x00\x60\x00\x00\x00\x36\x00\x00\x00" // ........`...6...
            "\x5c\x00\x50\x00\x72\x00\x6f\x00\x67\x00\x72\x00\x61\x00\x6d\x00" // ..P.r.o.g.r.a.m.
            "\x20\x00\x46\x00\x69\x00\x6c\x00\x65\x00\x73\x00\x5c\x00\x64\x00" //  .F.i.l.e.s...d.
            "\x65\x00\x73\x00\x6b\x00\x74\x00\x6f\x00\x70\x00\x2e\x00\x69\x00" // e.s.k.t.o.p...i.
            "\x6e\x00\x69\x00\x00\x00"                                         // n.i...
        ;
    InStream in_stream(in_data, sizeof(in_data) - 1);

    rdpdr::DeviceCreateRequest device_create_request;

    device_create_request.receive(in_stream);

    //device_create_request.log(LOG_INFO);

    char out_data[sizeof(in_data)];

    OutStream out_stream(out_data, sizeof(out_data));

    device_create_request.emit(out_stream);
    //LOG(LOG_INFO, "out_stream_size=%u", (unsigned)out_stream.get_offset());
    //hexdump(out_stream.get_data(), out_stream.get_offset());

    BOOST_CHECK_EQUAL(out_stream.get_offset(), in_stream.get_offset());
    BOOST_CHECK_EQUAL(0, memcmp(in_data, out_data, sizeof(in_data) - 1));
}

BOOST_AUTO_TEST_CASE(TestDeviceCreateResponse1)
{
    const char in_data[] =
            "\x00\x00\x00\x00\x00"                                             // .....
        ;
    InStream in_stream(in_data, sizeof(in_data) - 1);

    rdpdr::DeviceCreateResponse device_create_response;

    uint32_t IoStatus = 0x00000000;

    device_create_response.receive(in_stream, IoStatus);

    //device_create_request.log(LOG_INFO);

    char out_data[sizeof(in_data)];

    OutStream out_stream(out_data, sizeof(out_data));

    device_create_response.emit(out_stream);
    //LOG(LOG_INFO, "out_stream_size=%u", (unsigned)out_stream.get_offset());
    //hexdump(out_stream.get_data(), out_stream.get_offset());

    BOOST_CHECK_EQUAL(out_stream.get_offset(),
                      in_stream.get_offset() +
                      1 /* Information(1) is ignored */);
    BOOST_CHECK_EQUAL(0, memcmp(in_data, out_data, sizeof(in_data) - 1));
}

BOOST_AUTO_TEST_CASE(TestDeviceCreateResponse2)
{
    const char in_data[] =
            "\x00\x00\x00\x00"                                                 // ....
        ;
    InStream in_stream(in_data, sizeof(in_data) - 1);

    rdpdr::DeviceCreateResponse device_create_response;

    uint32_t IoStatus = 0x00000000;

    device_create_response.receive(in_stream, IoStatus);

    //device_create_request.log(LOG_INFO);

    char out_data[sizeof(in_data)];

    OutStream out_stream(out_data, sizeof(out_data));

    device_create_response.emit(out_stream);
    //LOG(LOG_INFO, "out_stream_size=%u", (unsigned)out_stream.get_offset());
    //hexdump(out_stream.get_data(), out_stream.get_offset());

    BOOST_CHECK_EQUAL(out_stream.get_offset(),
                      in_stream.get_offset() +
                      1 /* Information(1) is ignored */);
    BOOST_CHECK_EQUAL(0, memcmp(in_data, out_data, sizeof(in_data) - 1));
}

BOOST_AUTO_TEST_CASE(TestDeviceCreateResponse3)
{
    const char in_data[] =
            "\x00\x00\x00\x00\x01"                                             // .....
        ;
    InStream in_stream(in_data, sizeof(in_data) - 1);

    rdpdr::DeviceCreateResponse device_create_response;

    uint32_t IoStatus = 0xC0000022;

    device_create_response.receive(in_stream, IoStatus);

    //device_create_request.log(LOG_INFO);

    char out_data[sizeof(in_data)];

    OutStream out_stream(out_data, sizeof(out_data));

    device_create_response.emit(out_stream);
    //LOG(LOG_INFO, "out_stream_size=%u", (unsigned)out_stream.get_offset());
    //hexdump(out_stream.get_data(), out_stream.get_offset());

    BOOST_CHECK_EQUAL(out_stream.get_offset(), in_stream.get_offset());
    BOOST_CHECK_EQUAL(0, memcmp(in_data, out_data, sizeof(in_data) - 1));
}


BOOST_AUTO_TEST_CASE(ClientAnnounceReply) {

    StaticOutStream<16> out_stream;

    rdpdr::SharedHeader sharedHeader( rdpdr::Component::RDPDR_CTYP_CORE
                                    , rdpdr::PacketId::PAKID_CORE_CLIENTID_CONFIRM);
    sharedHeader.emit(out_stream);

    rdpdr::ClientAnnounceReply clientAnnounceReply(0x0001, 0x0002, 0x00000001);
    clientAnnounceReply.emit(out_stream);

    std::string out_data(reinterpret_cast<char *>(out_stream.get_data()), 12);

    const char expected_data[] =
        "\x72\x44\x43\x43\x01\x00\x02\x00\x01\x00\x00\x00";
    std::string expected(reinterpret_cast<const char *>(expected_data), 12);

    BOOST_CHECK_EQUAL(expected, out_data);
}


BOOST_AUTO_TEST_CASE(ClientNameRequest1)
{
    const char in_data[] =
            "\x01\x00\x00\x00\x00\x00\x00\x00\x08\x00\x00\x00\x72\x00\x7a\x00" // ............r.z.
            "\x68\x00\x00\x00"                                                 // h...
        ;
    InStream in_stream(in_data, sizeof(in_data) - 1);

    rdpdr::ClientNameRequest client_name_request;

    client_name_request.receive(in_stream);

    //client_name_request.log(LOG_INFO);

    char out_data[sizeof(in_data)];

    OutStream out_stream(out_data, sizeof(out_data));

    client_name_request.emit(out_stream);
    //LOG(LOG_INFO, "out_stream_size=%u", (unsigned)out_stream.get_offset());
    //hexdump(out_stream.get_data(), out_stream.get_offset());

    BOOST_CHECK_EQUAL(out_stream.get_offset(), in_stream.get_offset());
    BOOST_CHECK_EQUAL(0, memcmp(in_data, out_data, sizeof(in_data) - 1));
}

BOOST_AUTO_TEST_CASE(ClientNameRequest2)
{
    const char in_data[] =
            "\x4d\x62\x16\x2d\x00\x00\x00\x00\x12\x00\x00\x00\x52\x00\x44\x00" // Mb.-........R.D.
            "\x50\x00\x2d\x00\x54\x00\x45\x00\x53\x00\x54\x00\x00\x00"         // P.-.T.E.S.T...
        ;
    InStream in_stream(in_data, sizeof(in_data) - 1);

    rdpdr::ClientNameRequest client_name_request;

    client_name_request.receive(in_stream);

    //client_name_request.log(LOG_INFO);

    char out_data[sizeof(in_data)];

    OutStream out_stream(out_data, sizeof(out_data));

    client_name_request.emit(out_stream);
    //LOG(LOG_INFO, "out_stream_size=%u", (unsigned)out_stream.get_offset());
    //hexdump(out_stream.get_data(), out_stream.get_offset());

    BOOST_CHECK_EQUAL(out_stream.get_offset(), in_stream.get_offset());
    BOOST_CHECK_EQUAL(0, memcmp(in_data, out_data, sizeof(in_data) - 1));
}

BOOST_AUTO_TEST_CASE(FileRenameInformation1)
{
    const char in_data[] =
            "\x00\x00\x24\x00\x00\x00\x5c\x00\x57\x00\x41\x00\x42\x00\x41\x00" // ..&.....W.A.B.A.
            "\x67\x00\x65\x00\x6e\x00\x74\x00\x20\x00\x2d\x00\x20\x00\x43\x00" // g.e.n.t. .-. .C.
            "\x6f\x00\x2e\x00\x65\x00\x78\x00\x65\x00"                         // o...e.x.e.
        ;
    InStream in_stream(in_data, sizeof(in_data) - 1);

    rdpdr::RDPFileRenameInformation file_rename_information;

    file_rename_information.receive(in_stream);

    //file_rename_information.log(LOG_INFO);

    char out_data[sizeof(in_data)];

    OutStream out_stream(out_data, sizeof(out_data));

    file_rename_information.emit(out_stream);
    //LOG(LOG_INFO, "out_stream_size=%u", (unsigned)out_stream.get_offset());
    //hexdump(out_stream.get_data(), out_stream.get_offset());

    BOOST_CHECK_EQUAL(out_stream.get_offset(), in_stream.get_offset());
    BOOST_CHECK_EQUAL(0, memcmp(in_data, out_data, sizeof(in_data) - 1));
}

BOOST_AUTO_TEST_CASE(ServerDriveQueryDirectoryRequest1)
{
    const char in_data[] =
            "\x03\x00\x00\x00\x01\x0a\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
            "\x5c\x00\x42\x00\x49\x00\x4e\x00\x00\x00"                         // ..B.I.N...
        ;
    InStream in_stream(in_data, sizeof(in_data) - 1);

    rdpdr::ServerDriveQueryDirectoryRequest server_drive_query_directory_request;

    server_drive_query_directory_request.receive(in_stream);

    //server_drive_query_directory_request.log(LOG_INFO);

    char out_data[sizeof(in_data)];

    OutStream out_stream(out_data, sizeof(out_data));

    server_drive_query_directory_request.emit(out_stream);
    //LOG(LOG_INFO, "out_stream_size=%u", (unsigned)out_stream.get_offset());
    //hexdump(out_stream.get_data(), out_stream.get_offset());

    BOOST_CHECK_EQUAL(out_stream.get_offset(), in_stream.get_offset());
    BOOST_CHECK_EQUAL(0, memcmp(in_data, out_data, sizeof(in_data) - 1));
}

BOOST_AUTO_TEST_CASE(ServerDriveQueryDirectoryRequest2)
{
    const char in_data[] =
            "\x03\x00\x00\x00\x01\x2a\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // .....*..........
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
            "\x5c\x00\x53\x00\x70\x00\x79\x00\x5c\x00\x73\x00\x70\x00\x79\x00" // ..S.p.y...s.p.y.
            "\x78\x00\x78\x00\x5f\x00\x61\x00\x6d\x00\x64\x00\x36\x00\x34\x00" // x.x._.a.m.d.6.4.
            "\x2e\x00\x65\x00\x78\x00\x65\x00\x00\x00"                         // ..e.x.e...
        ;
    InStream in_stream(in_data, sizeof(in_data) - 1);

    rdpdr::ServerDriveQueryDirectoryRequest server_drive_query_directory_request;

    server_drive_query_directory_request.receive(in_stream);

    //server_drive_query_directory_request.log(LOG_INFO);

    char out_data[sizeof(in_data)];

    OutStream out_stream(out_data, sizeof(out_data));

    server_drive_query_directory_request.emit(out_stream);
    //LOG(LOG_INFO, "out_stream_size=%u", (unsigned)out_stream.get_offset());
    //hexdump(out_stream.get_data(), out_stream.get_offset());

    BOOST_CHECK_EQUAL(out_stream.get_offset(), in_stream.get_offset());
    BOOST_CHECK_EQUAL(0, memcmp(in_data, out_data, sizeof(in_data) - 1));
}
