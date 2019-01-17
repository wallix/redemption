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
    Author(s): Christophe Grosjean, Raphael Zhou,  Clément Moroldo
*/

#define RED_TEST_MODULE Test_rdpdr
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "core/RDP/channels/rdpdr.hpp"

RED_AUTO_TEST_CASE(TestDeviceCreateRequest1)
{
    const char in_data[] =
            "\x81\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
            "\x07\x00\x00\x00\x01\x00\x00\x00\x20\x00\x00\x00\x0e\x00\x00\x00" // ........ .......
            "\x5c\x00\x2e\x00\x63\x00\x70\x00\x61\x00\x6e\x00\x00\x00"         // ....c.p.a.n...
        ;
    InStream in_stream(in_data, sizeof(in_data) - 1);

    rdpdr::DeviceCreateRequest device_create_request;

    device_create_request.receive(in_stream);

    RED_CHECK_EQUAL(device_create_request.DesiredAccess(), 0x00100081);
    RED_CHECK_EQUAL(device_create_request.AllocationSize(), 0);
    RED_CHECK_EQUAL(device_create_request.FileAttributes(), 0);
    RED_CHECK_EQUAL(device_create_request.SharedAccess(), 7);
    RED_CHECK_EQUAL(device_create_request.CreateDisposition(), 1);
    RED_CHECK_EQUAL(device_create_request.CreateOptions(), 32);
    RED_CHECK_EQUAL(device_create_request.PathLength(), 14);
    RED_CHECK_EQUAL(0, memcmp(device_create_request.Path().data(), "/.cpan", (device_create_request.PathLength()/2)-1));

    //device_create_request.log();

    char out_data[sizeof(in_data)];

    OutStream out_stream(out_data, sizeof(out_data));

    device_create_request.emit(out_stream);
    //LOG(LOG_INFO, "out_stream_size=%u", (unsigned)out_stream.get_offset());
    //hexdump_av(out_stream.get_bytes())

    RED_CHECK_EQUAL(out_stream.get_offset(), in_stream.get_offset());
    RED_CHECK_EQUAL(0, memcmp(in_data, out_data, sizeof(in_data) - 1));
}

RED_AUTO_TEST_CASE(TestDeviceCreateRequest2)
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

    RED_CHECK_EQUAL(device_create_request.DesiredAccess(), 0x00120089);
    RED_CHECK_EQUAL(device_create_request.AllocationSize(), 0);
    RED_CHECK_EQUAL(device_create_request.FileAttributes(), 0);
    RED_CHECK_EQUAL(device_create_request.SharedAccess(), 7);
    RED_CHECK_EQUAL(device_create_request.CreateDisposition(), 1);
    RED_CHECK_EQUAL(device_create_request.CreateOptions(), 0x00000060);
    RED_CHECK_EQUAL(device_create_request.PathLength(), 54);
    RED_CHECK_EQUAL(0, memcmp(device_create_request.Path().data(), "/Program Files/desktop.ini", (device_create_request.PathLength()/2)-1));

    //device_create_request.log();

    char out_data[sizeof(in_data)];

    OutStream out_stream(out_data, sizeof(out_data));

    device_create_request.emit(out_stream);
    //LOG(LOG_INFO, "out_stream_size=%u", (unsigned)out_stream.get_offset());
    //hexdump_av(out_stream.get_bytes())

    RED_CHECK_EQUAL(out_stream.get_offset(), in_stream.get_offset());
    RED_CHECK_EQUAL(0, memcmp(in_data, out_data, sizeof(in_data) - 1));
}

RED_AUTO_TEST_CASE(TestDeviceCreateResponse1)
{
    const char in_data[] =
            "\x00\x00\x00\x00\x00"                                             // .....
        ;
    InStream in_stream(in_data, sizeof(in_data) - 1);

    rdpdr::DeviceCreateResponse device_create_response;

    erref::NTSTATUS IoStatus = erref::NTSTATUS::STATUS_SUCCESS;

    device_create_response.receive(in_stream, IoStatus);

    //device_create_request.log(LOG_INFO);

    char out_data[sizeof(in_data)];

    OutStream out_stream(out_data, sizeof(out_data));

    device_create_response.emit(out_stream);
    //LOG(LOG_INFO, "out_stream_size=%u", (unsigned)out_stream.get_offset());
    //hexdump_av(out_stream.get_bytes())

    RED_CHECK_EQUAL(out_stream.get_offset(),
                      in_stream.get_offset() +
                      1 /* Information(1) is ignored */);
    RED_CHECK_EQUAL(0, memcmp(in_data, out_data, sizeof(in_data) - 1));
}

RED_AUTO_TEST_CASE(TestDeviceCreateResponse2)
{
    const char in_data[] =
            "\x00\x00\x00\x00"                                                 // ....
        ;
    InStream in_stream(in_data, sizeof(in_data) - 1);

    rdpdr::DeviceCreateResponse device_create_response;

    erref::NTSTATUS IoStatus = erref::NTSTATUS::STATUS_SUCCESS;

    device_create_response.receive(in_stream, IoStatus);

    //device_create_request.log(LOG_INFO);

    char out_data[sizeof(in_data)];

    OutStream out_stream(out_data, sizeof(out_data));

    device_create_response.emit(out_stream);
    //LOG(LOG_INFO, "out_stream_size=%u", (unsigned)out_stream.get_offset());
    //hexdump_av(out_stream.get_bytes())

    RED_CHECK_EQUAL(out_stream.get_offset(),
                      in_stream.get_offset() +
                      1 /* Information(1) is ignored */);
    RED_CHECK_EQUAL(0, memcmp(in_data, out_data, sizeof(in_data) - 1));
}

RED_AUTO_TEST_CASE(TestDeviceCreateResponse3)
{
    const char in_data[] =
            "\x00\x00\x00\x00\x01"                                             // .....
        ;
    InStream in_stream(in_data, sizeof(in_data) - 1);

    rdpdr::DeviceCreateResponse device_create_response;

    erref::NTSTATUS IoStatus = erref::NTSTATUS::STATUS_ACCESS_DENIED;

    device_create_response.receive(in_stream, IoStatus);

    //device_create_request.log(LOG_INFO);

    char out_data[sizeof(in_data)];

    OutStream out_stream(out_data, sizeof(out_data));

    device_create_response.emit(out_stream);
    //LOG(LOG_INFO, "out_stream_size=%u", (unsigned)out_stream.get_offset());
    //hexdump_av(out_stream.get_bytes())

    RED_CHECK_EQUAL(out_stream.get_offset(), in_stream.get_offset());
    RED_CHECK_EQUAL(0, memcmp(in_data, out_data, sizeof(in_data) - 1));
}


RED_AUTO_TEST_CASE(ClientAnnounceReply) {

    StaticOutStream<16> out_stream;

    rdpdr::SharedHeader sharedHeader( rdpdr::Component::RDPDR_CTYP_CORE
                                    , rdpdr::PacketId::PAKID_CORE_CLIENTID_CONFIRM);
    sharedHeader.emit(out_stream);

    rdpdr::ClientAnnounceReply clientAnnounceReply(0x0001, 0x0002, 0x00000001);
    clientAnnounceReply.emit(out_stream);

    auto expected_data = cstr_array_view("\x72\x44\x43\x43\x01\x00\x02\x00\x01\x00\x00\x00");
    RED_CHECK_MEM(expected_data, out_stream.get_bytes());
}


RED_AUTO_TEST_CASE(ClientNameRequest1)
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
    //hexdump_av(out_stream.get_bytes())

    RED_CHECK_EQUAL(out_stream.get_offset(), in_stream.get_offset());
    RED_CHECK_EQUAL(0, memcmp(in_data, out_data, sizeof(in_data) - 1));
}

RED_AUTO_TEST_CASE(ClientNameRequest2)
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
    //hexdump_av(out_stream.get_bytes())

    RED_CHECK_EQUAL(out_stream.get_offset(), in_stream.get_offset());
    RED_CHECK_EQUAL(0, memcmp(in_data, out_data, sizeof(in_data) - 1));
}

RED_AUTO_TEST_CASE(FileRenameInformation1)
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
    //hexdump_av(out_stream.get_bytes())

    RED_CHECK_EQUAL(out_stream.get_offset(), in_stream.get_offset());
    RED_CHECK_EQUAL(0, memcmp(in_data, out_data, sizeof(in_data) - 1));
}

RED_AUTO_TEST_CASE(ServerDriveQueryDirectoryRequest1)
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
    //hexdump_av(out_stream.get_bytes())

    RED_CHECK_EQUAL(out_stream.get_offset(), in_stream.get_offset());
    RED_CHECK_EQUAL(0, memcmp(in_data, out_data, sizeof(in_data) - 1));
}

RED_AUTO_TEST_CASE(ServerDriveQueryDirectoryRequest2)
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
    //hexdump_av(out_stream.get_bytes())

    RED_CHECK_EQUAL(out_stream.get_offset(), in_stream.get_offset());
    RED_CHECK_EQUAL(0, memcmp(in_data, out_data, sizeof(in_data) - 1));
}

RED_AUTO_TEST_CASE(CapabilityHeader)
{
    auto data = "\x01\x00\x08\x00\x01\x00\x00\x00"_av;

    {
        StaticOutStream<8> stream;
        rdpdr::CapabilityHeader ch(0x01, 0x0001);

        ch.emit(stream);

        RED_CHECK_MEM(stream.get_bytes(), data);
    }

    {
        InStream in_stream(data);
        rdpdr::CapabilityHeader ch;

        ch.receive(in_stream);

        RED_CHECK_EQUAL(ch.CapabilityType, 0x01);
        RED_CHECK_EQUAL(ch.CapabilityLength, 0x08);
        RED_CHECK_EQUAL(ch.Version, 0x0001);
    }
}


RED_AUTO_TEST_CASE(ClientDriveDeviceListRemove)
{
    auto data = "\x03\x00\x00\x00\x01\x00\x00\x00\x02\x00\x00\x00\x03\x00\x00\x00"_av;

    {
        StaticOutStream<128> stream;
        uint32_t DeviceIds[] = {1, 2, 3};
        rdpdr::ClientDriveDeviceListRemove cddlr(03, DeviceIds);

        cddlr.emit(stream);

        RED_CHECK_MEM(stream.get_bytes(), data);
    }

    {
        InStream in_stream(data);
        rdpdr::ClientDriveDeviceListRemove cddlr;

        cddlr.receive(in_stream);

        RED_CHECK_EQUAL(cddlr.DeviceCount, 3);
        RED_CHECK_EQUAL(cddlr.DeviceIds[0], 1);
        RED_CHECK_EQUAL(cddlr.DeviceIds[1], 2);
        RED_CHECK_EQUAL(cddlr.DeviceIds[2], 3);
    }
}

RED_AUTO_TEST_CASE(DeviceAnnounceHeader)
{
    auto data =
        "\x20\x00\x00\x00\x01\x00\x00\x00\x53\x43\x41\x52\x44\x00\x00\x00"
        "\x00\x00\x00\x00"_av;

    {
        StaticOutStream<128> stream;
        uint8_t device_data[1] {};
        rdpdr::DeviceAnnounceHeader pdu(rdpdr::RDPDR_DTYP_SMARTCARD, 01, "SCARD", device_data, 0);

        pdu.emit(stream);

        RED_CHECK_MEM(stream.get_bytes(), data);
    }

    {
        InStream in_stream(data);
        rdpdr::DeviceAnnounceHeader pdu;

        pdu.receive(in_stream);

        RED_CHECK_EQUAL(pdu.DeviceType(), 0x00000020);
        RED_CHECK_EQUAL(pdu.DeviceId(), 01);
        RED_CHECK_EQUAL(pdu.PreferredDosName(), "SCARD");
        RED_CHECK_EQUAL(pdu.DeviceDataLength(), 0);
        RED_CHECK_EQUAL(pdu.DeviceData(), nullptr);
    }
}

RED_AUTO_TEST_CASE(DeviceIORequest)
{
    auto data =
        "\x01\x00\x00\x00\x02\x00\x00\x00\x03\x00\x00\x00\01\x00\x00\x00"
        "\x01\x00\x00\x00"_av;

    {
        StaticOutStream<128> stream;
        rdpdr::DeviceIORequest pdu(01, 02, 03, smb2::FILE_OPEN, smb2::FILE_DIRECTORY_FILE);

        pdu.emit(stream);

        RED_CHECK_MEM(stream.get_bytes(), data);
    }

    {
        InStream in_stream(data);
        rdpdr::DeviceIORequest pdu;

        pdu.receive(in_stream);

        RED_CHECK_EQUAL(pdu.DeviceId(), 01);
        RED_CHECK_EQUAL(pdu.FileId(), 02);
        RED_CHECK_EQUAL(pdu.CompletionId(), 03);
        RED_CHECK_EQUAL(pdu.MajorFunction(), smb2::FILE_OPEN);
        RED_CHECK_EQUAL(pdu.MinorFunction(), smb2::FILE_DIRECTORY_FILE);
    }
}


/*RED_AUTO_TEST_CASE(DeviceCloseRequestEmit)
{
    const size_t len = 32;
    const char data[] =
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

    StaticOutStream<128> stream;
    rdpdr::DeviceIORequest pdu;
    pdu.emit(stream);

    std::string const out_data(data, len);
    std::string const expected(char_ptr_cast(stream.get_data()), len);
    for (int i = 0; i < 32; i++) {
        std::cout << int(data[i]) <<  " " << int(stream.get_data()[i]) << std::endl;
    }
    RED_CHECK_EQUAL(expected, out_data);
}*/


RED_AUTO_TEST_CASE(ClientDriveNotifyChangeDirectoryResponse)
{
    auto data = "\xff\x07\xff\x07"_av;

    {
        StaticOutStream<128> stream;
        rdpdr::ClientDriveNotifyChangeDirectoryResponse pdu(0x07ff07ff);

        pdu.emit(stream);

        RED_CHECK_MEM(stream.get_bytes(), data);
    }

    {
        InStream in_stream(data);
        rdpdr::ClientDriveNotifyChangeDirectoryResponse pdu;
        pdu.receive(in_stream);

        RED_CHECK_EQUAL(pdu.Length, 0x07ff07ff);
    }
}

RED_AUTO_TEST_CASE(ServerDriveNotifyChangeDirectoryRequest)
{
    auto data = "\x01\xff\x07\xff\x07"_av;

    {
        StaticOutStream<128> stream;
        rdpdr::ServerDriveNotifyChangeDirectoryRequest pdu(01, 0x07ff07ff);

        pdu.emit(stream);

        RED_CHECK_MEM(stream.get_bytes(), data);
    }

    {
        InStream in_stream(data);
        rdpdr::ServerDriveNotifyChangeDirectoryRequest pdu;

        pdu.receive(in_stream);

        RED_CHECK_EQUAL(pdu.WatchTree, 01);
        RED_CHECK_EQUAL(pdu.CompletionFilter, 0x07ff07ff);
    }
}

RED_AUTO_TEST_CASE(RDP_Lock_Info)
{
    auto data = "\x01\x00\xff\x00\xff\x00\xff\x02\x03\x00\xff\x00\xff\x00\xff\x04"_av;

    {
        StaticOutStream<128> stream;
        rdpdr::RDP_Lock_Info pdu(0x02ff00ff00ff0001, 0x04ff00ff00ff0003);

        pdu.emit(stream);

        RED_CHECK_MEM(stream.get_bytes(), data);
    }

    {
        InStream in_stream(data);
        rdpdr::RDP_Lock_Info pdu;
        pdu.receive(in_stream);

        RED_CHECK_EQUAL(pdu.Length, 0x02ff00ff00ff0001);
        RED_CHECK_EQUAL(pdu.Offset, 0x04ff00ff00ff0003);
    }
}

RED_AUTO_TEST_CASE(streamLog)
{
    const char data[] =
    /* 0000 */ "\x01\x00\x00\x00"                                                 //....
    /* 0000 */ "\x00\x00\x00\x00"                                                 //....
    /* 0000 */ "\x03\x00\x00\x00"                                                 //....
    /* 0000 */ "\x54\x00\x00\x00"                                                 //T...
    /* 0000 */ "\x72\x44\x50\x43\x05\x00\x00\x00\x01\x00\x2c\x00\x02\x00\x00\x00" //rDPC......,.....
    /* 0010 */ "\x02\x00\x00\x00\x01\x00\x06\x00\x01\x00\x0c\x00\xff\xff\x00\x00" //................
    /* 0020 */ "\x00\x00\x00\x00\x07\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00" //................
    /* 0030 */ "\x00\x00\x00\x00\x02\x00\x08\x00\x01\x00\x00\x00\x03\x00\x08\x00" //................
    /* 0040 */ "\x01\x00\x00\x00\x04\x00\x08\x00\x01\x00\x00\x00\x05\x00\x08\x00" //................
    /* 0050 */ "\x01\x00\x00\x00";

    InStream in_stream(data);

    rdpdr::RdpDrStatus status;

    rdpdr::streamLog(in_stream, status);

    // TODO TEST this not a test
}

RED_AUTO_TEST_CASE(ServerDriveLockControlRequest)
{
    auto data =
        "\x01\x00\x00\x02\x80\x00\x00\x00\x03\x00\x00\x04\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"_av;

    {
        StaticOutStream<128> stream;
        rdpdr::ServerDriveLockControlRequest pdu(0x02000001, 1, 0x04000003);

        pdu.emit(stream);

        RED_CHECK_MEM(stream.get_bytes(), data);
    }

    {
        InStream in_stream(data);
        rdpdr::ServerDriveLockControlRequest pdu;

        pdu.receive(in_stream);

        RED_CHECK_EQUAL(pdu.Operation, 0x02000001);
        RED_CHECK_EQUAL(pdu.F, 1);
        RED_CHECK_EQUAL(pdu.NumLocks, 0x04000003);
    }
}

RED_AUTO_TEST_CASE(ClientDriveSetInformationResponse)
{
    auto data = "\x01\x00\x00\x02\x00"_av;

    {
        StaticOutStream<128> stream;
        rdpdr::ClientDriveSetInformationResponse pdu(0x02000001);

        pdu.emit(stream);

        RED_CHECK_MEM(stream.get_bytes(), data);
    }

    {
        InStream in_stream(data);
        rdpdr::ClientDriveSetInformationResponse pdu;
        pdu.receive(in_stream);

        RED_CHECK_EQUAL(pdu.Length, 0x02000001);
    }
}

RED_AUTO_TEST_CASE(ServerDriveSetVolumeInformationRequest)
{
    auto data =
        "\x01\x00\x00\x02\x03\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"_av;

    {
        StaticOutStream<128> stream;
        rdpdr::ServerDriveSetVolumeInformationRequest pdu(0x02000001, 0x04000003);

        pdu.emit(stream);

        RED_CHECK_MEM(stream.get_bytes(), data);
    }

    {
        InStream in_stream(data);
        rdpdr::ServerDriveSetVolumeInformationRequest pdu;

        pdu.receive(in_stream);

        RED_CHECK_EQUAL(pdu.FsInformationClass, 0x02000001);
        RED_CHECK_EQUAL(pdu.Length, 0x04000003);
    }
}

RED_AUTO_TEST_CASE(ClientDriveQueryDirectoryResponseRequest)
{
    auto data = "\x01\x00\x00\x02"_av;

    {
        StaticOutStream<128> stream;
        rdpdr::ClientDriveQueryDirectoryResponse pdu(0x02000001);

        pdu.emit(stream);

        RED_CHECK_MEM(stream.get_bytes(), data);
    }

    {
        InStream in_stream(data);
        rdpdr::ClientDriveQueryDirectoryResponse pdu;
        pdu.receive(in_stream);

        RED_CHECK_EQUAL(pdu.Length, 0x02000001);
    }
}

RED_AUTO_TEST_CASE(ServerDriveSetInformationRequest)
{
    auto data = "\x01\x00\x00\x02\x03\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00"
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"_av;

    {
        StaticOutStream<128> stream;
        rdpdr::ServerDriveSetInformationRequest pdu(0x02000001, 0x04000003);

        pdu.emit(stream);

        RED_CHECK_MEM(stream.get_bytes(), data);
    }

    {
        InStream in_stream(data);
        rdpdr::ServerDriveSetInformationRequest pdu;

        pdu.receive(in_stream);

        RED_CHECK_EQUAL(pdu.FsInformationClass(), 0x02000001);
        RED_CHECK_EQUAL(pdu.Length(), 0x04000003);
    }
}

RED_AUTO_TEST_CASE(ClientDriveQueryVolumeInformationResponse)
{
    auto data = "\x01\x00\x00\x02"_av;

    {
        StaticOutStream<128> stream;
        rdpdr::ClientDriveQueryVolumeInformationResponse pdu(0x02000001);

        pdu.emit(stream);

        RED_CHECK_MEM(stream.get_bytes(), data);
    }

    {
        InStream in_stream(data);
        rdpdr::ClientDriveQueryVolumeInformationResponse pdu;
        pdu.receive(in_stream);

        RED_CHECK_EQUAL(pdu.Length, 0x02000001);
    }
}

RED_AUTO_TEST_CASE(ClientDeviceListAnnounceRequest)
{
    auto data = "\x0a\x00\x00\x00"_av;

    {
        StaticOutStream<128> stream;
        rdpdr::ClientDeviceListAnnounceRequest pdu(10);

        pdu.emit(stream);

        RED_CHECK_MEM(stream.get_bytes(), data);
    }

    {
        InStream in_stream(data);
        rdpdr::ClientDeviceListAnnounceRequest pdu;

        pdu.receive(in_stream);

        RED_CHECK_EQUAL(pdu.DeviceCount, 0x0a);
    }
}

RED_AUTO_TEST_CASE(GeneralCapabilitySet)
{
    auto data =
        "\x02\x00\x00\x00\x02\x00\x00\x00\x03\x00\x02\x00\x05\x00\x00\x00"
        "\x06\x00\x00\x00\x07\x00\x00\x00\x08\x00\x00\x00\x09\x00\x00\x00"
        "\x00\x00\x00\x00"_av
        ;
    {
        StaticOutStream<128> stream;
        rdpdr::GeneralCapabilitySet pdu(0x2,        // osType
                                        rdpdr::MINOR_VERSION_2,        // protocolMinorVersion -
                                        rdpdr::SUPPORT_ALL_REQUEST,     // ioCode1
                                        0x00000007,
                                        rdpdr::ENABLE_ASYNCIO,        // extraFlags1
                                        0,                          // SpecialTypeDeviceCap
                                        rdpdr::GENERAL_CAPABILITY_VERSION_02);
        pdu.osVersion = 2;
        pdu.protocolMajorVersion = 3;
        pdu.ioCode1 = 5;
        pdu.ioCode2 = 6;
        pdu.extraFlags1_ = 8;
        pdu.extraFlags2 = 9;
        pdu.emit(stream);

        RED_CHECK_MEM(stream.get_bytes(), data);
    }

    {
        InStream in_stream(data);
        rdpdr::GeneralCapabilitySet pdu;

        pdu.receive(in_stream, rdpdr::GENERAL_CAPABILITY_VERSION_02);

        RED_CHECK_EQUAL(pdu.osType, 2);
        RED_CHECK_EQUAL(pdu.osVersion, 2);
        RED_CHECK_EQUAL(pdu.protocolMajorVersion, 3);
        RED_CHECK_EQUAL(pdu.protocolMinorVersion, uint16_t(rdpdr::MINOR_VERSION_2));
        RED_CHECK_EQUAL(pdu.ioCode1, 5);
        RED_CHECK_EQUAL(pdu.ioCode2, 6);
        RED_CHECK_EQUAL(pdu.extendedPDU_, 7);
        RED_CHECK_EQUAL(pdu.extraFlags1_, 8);
        RED_CHECK_EQUAL(pdu.extraFlags2, 9);
        RED_CHECK_EQUAL(pdu.SpecialTypeDeviceCap, 0);
    }
}

RED_AUTO_TEST_CASE(ServerAnnounceRequest)
{
    auto data = "\x01\x00\x02\x00\x03\x00\x00\x00"_av;

    {
        StaticOutStream<128> stream;
        rdpdr::ServerAnnounceRequest pdu(1, 2, 3);

        pdu.emit(stream);

        RED_CHECK_MEM(stream.get_bytes(), data);
    }

    {
        InStream in_stream(data);
        rdpdr::ServerAnnounceRequest pdu;

        pdu.receive(in_stream);

        RED_CHECK_EQUAL(pdu.VersionMajor(), 1);
        RED_CHECK_EQUAL(pdu.VersionMinor(), 2);
        RED_CHECK_EQUAL(pdu.ClientId(), 3);
    }
}

RED_AUTO_TEST_CASE(ServerDeviceAnnounceResponse)
{
    auto data = "\x01\x00\x00\x00\x01\x00\x00\xc0"_av;

    {
        StaticOutStream<128> stream;
        rdpdr::ServerDeviceAnnounceResponse pdu(1, erref::NTSTATUS::STATUS_UNSUCCESSFUL);

        pdu.emit(stream);

        RED_CHECK_MEM(stream.get_bytes(), data);
    }

    {
        InStream in_stream(data);
        rdpdr::ServerDeviceAnnounceResponse pdu;

        pdu.receive(in_stream);

        RED_CHECK_EQUAL(pdu.DeviceId(), 1);
        RED_CHECK_EQUAL(uint32_t(pdu.ResultCode()), uint32_t(erref::NTSTATUS::STATUS_UNSUCCESSFUL));
    }
}

RED_AUTO_TEST_CASE(DeviceWriteResponse)
{
    auto data = "\x01\x00\x00\x00\x00"_av;

    {
        StaticOutStream<128> stream;
        rdpdr::DeviceWriteResponse pdu(1);

        pdu.emit(stream);

        RED_CHECK_MEM(stream.get_bytes(), data);
    }

    {
        InStream in_stream(data);
        rdpdr::DeviceWriteResponse pdu;

        pdu.receive(in_stream);

        RED_CHECK_EQUAL(pdu.Length, 1);
    }
}

RED_AUTO_TEST_CASE(DeviceReadResponse)
{
    auto data = "\x04\x00\x00\x00"_av;

    {
        StaticOutStream<128> stream;
        //uint8_t readData[] = {1, 2, 3, 4};
        rdpdr::DeviceReadResponse pdu(4);

        pdu.emit(stream);

        RED_CHECK_MEM(stream.get_bytes(), data);
    }

    {
        InStream in_stream(data);
        rdpdr::DeviceReadResponse pdu;
        pdu.receive(in_stream);

        RED_CHECK_EQUAL(pdu.Length, 4);
    }
}
