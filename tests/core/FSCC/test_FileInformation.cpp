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

#define RED_TEST_MODULE TestGCC
#include "test_only/test_framework/redemption_unit_tests.hpp"


#include "core/FSCC/FileInformation.hpp"

RED_AUTO_TEST_CASE(TestFileBothDirectoryInformation)
{
    const char in_data[] =
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xcf\x70\x3a\x49\xd1\x01" // ...........p:I..
            "\x00\x49\x4e\x80\x3a\x49\xd1\x01\x00\x00\xcf\x70\x3a\x49\xd1\x01" // .IN.:I.....p:I..
            "\x00\x00\xcf\x70\x3a\x49\xd1\x01\x00\xa2\x1f\x00\x00\x00\x00\x00" // ...p:I..........
            "\x00\xb0\x1f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x06\x00\x00\x00" // ................
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x42\x00\x49" // .............B.I
            "\x00\x4e\x00"                                                     // .N...
        ;
    InStream in_stream(in_data, sizeof(in_data) - 1);

    fscc::FileBothDirectoryInformation file_both_directory_information;

    file_both_directory_information.receive(in_stream);

    //file_both_directory_information.log();

    RED_CHECK_EQUAL(sizeof(in_data) - 1, file_both_directory_information.size());

    char out_data[sizeof(in_data)];

    OutStream out_stream(out_data, sizeof(out_data));

    file_both_directory_information.emit(out_stream);
    //LOG(LOG_INFO, "out_stream_size=%u", (unsigned)out_stream.get_offset());
    //hexdump_av(out_stream.get_bytes());

//     InStream in_test_stream(out_stream.get_bytes());
//     fscc::FileBothDirectoryInformation fbdi;
//     fbdi.receive(in_test_stream);
//     fbdi.log();

    RED_CHECK_EQUAL(out_stream.get_offset(), in_stream.get_offset());
    RED_CHECK_EQUAL(0, memcmp(in_data, out_data, sizeof(in_data) - 1));
}

RED_AUTO_TEST_CASE(TestFileBothDirectoryInformation1)
{
    const char in_data[] =
            "\x00\x00\x00\x00\x00\x00\x00\x00\x50\xea\xef\xf9\xe4\x20\xce\x01" // ........P.... ..
            "\xa0\x27\xf5\x05\x2b\x4a\xd1\x01\x00\x7f\xa7\x9f\x53\x21\xce\x01" // .'..+J......S!..
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
            "\x00\x00\x00\x00\x00\x00\x00\x00\x16\x00\x00\x00\x32\x00\x00\x00" // ............2...
            "\x00\x00\x00\x00\x10\x53\x00\x59\x00\x53\x00\x54\x00\x45\x00\x4d" // .....S.Y.S.T.E.M
            "\x00\x7e\x00\x31\x00\x00\x00\x00\x00\x00\x00\x00\x00\x53\x00\x79" // .~.1.........S.y
            "\x00\x73\x00\x74\x00\x65\x00\x6d\x00\x20\x00\x56\x00\x6f\x00\x6c" // .s.t.e.m. .V.o.l
            "\x00\x75\x00\x6d\x00\x65\x00\x20\x00\x49\x00\x6e\x00\x66\x00\x6f" // .u.m.e. .I.n.f.o
            "\x00\x72\x00\x6d\x00\x61\x00\x74\x00\x69\x00\x6f\x00\x6e\x00"     //.r.m.a.t.i.o.n.
        ;
    InStream in_stream(in_data, sizeof(in_data) - 1);

    fscc::FileBothDirectoryInformation file_both_directory_information;

    file_both_directory_information.receive(in_stream);

    //file_both_directory_information.log();

    //RED_CHECK_EQUAL(sizeof(in_data) - 1, file_both_directory_information.total_size());

    char out_data[sizeof(in_data)];

    OutStream out_stream(out_data, sizeof(out_data));

    file_both_directory_information.emit(out_stream);
    //LOG(LOG_INFO, "out_stream_size=%u", (unsigned)out_stream.get_offset());
    //hexdump_av(out_stream.get_bytes());
    //hexdump_av(in_stream.get_bytes());

//     InStream in_test_stream(out_stream.get_bytes());
//     fscc::FileBothDirectoryInformation fbdi;
//     fbdi.receive(in_test_stream);
//     fbdi.log();

    RED_CHECK_EQUAL(out_stream.get_offset(), in_stream.get_offset());
    RED_CHECK_EQUAL(0, memcmp(in_data, out_data, sizeof(in_data) - 1));
}


RED_AUTO_TEST_CASE(TestFileFullDirectoryInformation1)
{
    const char in_data[] =
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x91\x70\x1b\xee\x4e\xd1\x01" // ..........p..N..
            "\x00\xb7\x64\x66\xa2\x4e\xd1\x01\x00\x91\x70\x1b\xee\x4e\xd1\x01" // ..df.N....p..N..
            "\x00\x18\x04\x1f\xee\x4e\xd1\x01\x00\xa4\x1f\x00\x00\x00\x00\x00" // .....N..........
            "\x00\xc0\x1f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x06\x00\x00\x00" // ................
            "\x00\x00\x00\x00\x42\x00\x49\x00\x4e\x00"                         // ....B.I.N.
        ;
    InStream in_stream(in_data, sizeof(in_data) - 1);

    fscc::FileFullDirectoryInformation file_full_directory_information;

    file_full_directory_information.receive(in_stream);

    //file_full_directory_information.log(LOG_INFO);

    RED_CHECK_EQUAL(sizeof(in_data) - 1, file_full_directory_information.size());

    char out_data[sizeof(in_data)];

    OutStream out_stream(out_data, sizeof(out_data));

    file_full_directory_information.emit(out_stream);
    //LOG(LOG_INFO, "out_stream_size=%u", (unsigned)out_stream.get_offset());
    //hexdump_av(out_stream.get_bytes());

    RED_CHECK_EQUAL(out_stream.get_offset(), in_stream.get_offset());
    RED_CHECK_EQUAL(0, memcmp(in_data, out_data, sizeof(in_data) - 1));
}

RED_AUTO_TEST_CASE(TestFileFsAttributeInformation)
{
    const char in_data[] =
            "\x07\x00\x00\x00\xff\x00\x00\x00\x0a\x00\x00\x00\x46\x00\x41\x00" // ............F.A.
            "\x54\x00\x33\x00\x32\x00"                                         // T.3.2.
        ;
    InStream in_stream(in_data, sizeof(in_data) - 1);

    fscc::FileFsAttributeInformation file_fs_attribute_information;

    file_fs_attribute_information.receive(in_stream);

    //file_fs_attribute_information.log(LOG_INFO);

    RED_CHECK_EQUAL(sizeof(in_data) - 1, file_fs_attribute_information.size());

    char out_data[sizeof(in_data)];

    OutStream out_stream(out_data, sizeof(out_data));

    file_fs_attribute_information.emit(out_stream);
    //LOG(LOG_INFO, "out_stream_size=%u", (unsigned)out_stream.get_offset());
    //hexdump_av(out_stream.get_bytes());

    RED_CHECK_EQUAL(out_stream.get_offset(), in_stream.get_offset());
    RED_CHECK_EQUAL(0, memcmp(in_data, out_data, sizeof(in_data) - 1));
}

RED_AUTO_TEST_CASE(TestFileFsAttributeInformation1)
{
    const char in_data[] =
            "\xff\x00\x07\x00\xff\x00\x00\x00\x08\x00\x00\x00\x4e\x00\x54\x00" // ............N.T.
            "\x46\x00\x53\x00"                                     // F.S.
        ;
    InStream in_stream(in_data, sizeof(in_data) - 1);

    fscc::FileFsAttributeInformation file_fs_attribute_information;

    file_fs_attribute_information.receive(in_stream);

    //file_fs_attribute_information.log(LOG_INFO);

    RED_CHECK_EQUAL(sizeof(in_data) - 1, file_fs_attribute_information.size());

    char out_data[sizeof(in_data)];

    OutStream out_stream(out_data, sizeof(out_data));

    file_fs_attribute_information.emit(out_stream);
    //LOG(LOG_INFO, "out_stream_size=%u", (unsigned)out_stream.get_offset());
    //hexdump_av(out_stream.get_bytes());

    RED_CHECK_EQUAL(out_stream.get_offset(), in_stream.get_offset());
    RED_CHECK_EQUAL(0, memcmp(in_data, out_data, sizeof(in_data) - 1));
}

RED_AUTO_TEST_CASE(TestFileFsVolumeInformation1)
{
    const char in_data[] =
            "\x80\xc3\x0f\x2f\x72\x4c\xd1\x01\xc6\x8c\x17\x36\x0e\x00\x00\x00" // .../rL.....6....
            "\x00\x46\x00\x52\x00\x45\x00\x45\x00\x52\x00\x44\x00\x50\x00"     // .F.R.E.E.R.D.P.
        ;
    InStream in_stream(in_data, sizeof(in_data) - 1);

    fscc::FileFsVolumeInformation file_fs_volume_information;

    file_fs_volume_information.receive(in_stream);

    //file_fs_volume_information.log(LOG_INFO);

    RED_CHECK_EQUAL(sizeof(in_data) - 1, file_fs_volume_information.size());

    char out_data[sizeof(in_data)];

    OutStream out_stream(out_data, sizeof(out_data));

    file_fs_volume_information.emit(out_stream);
    //LOG(LOG_INFO, "out_stream_size=%u", (unsigned)out_stream.get_offset());
    //hexdump_av(out_stream.get_bytes());

    RED_CHECK_EQUAL(out_stream.get_offset(), in_stream.get_offset());
    RED_CHECK_EQUAL(0, memcmp(in_data, out_data, sizeof(in_data) - 1));
}


RED_AUTO_TEST_CASE(TestFileObjectBuffer_Type1)
{
    const char in_data[] =
                        "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //@...............
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
        "\x00\x00\x00\x00"                                                 //....

        ;
    InStream in_stream(in_data, sizeof(in_data) - 1);

    fscc::FileObjectBuffer_Type1 fileObjectBuffer_Type1;

    fileObjectBuffer_Type1.receive(in_stream);

    //fileObjectBuffer_Type1.log();

    RED_CHECK_EQUAL(sizeof(in_data) - 1, fileObjectBuffer_Type1.size());

    char out_data[sizeof(in_data)];

    OutStream out_stream(out_data, sizeof(out_data));

    fileObjectBuffer_Type1.emit(out_stream);
    //LOG(LOG_INFO, "out_stream_size=%u", (unsigned)out_stream.get_offset());
    //hexdump_av(out_stream.get_bytes());

    RED_CHECK_EQUAL(out_stream.get_offset(), in_stream.get_offset());
    RED_CHECK_EQUAL(0, memcmp(in_data, out_data, sizeof(in_data) - 1));
}

RED_AUTO_TEST_CASE(TestFileObjectBuffer_Type2)
{
    const char in_data[] =
                        "\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //@...............
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
        "\x00\x00\x00\x00"                                                 //....

        ;
    InStream in_stream(in_data, sizeof(in_data) - 1);

    fscc::FileObjectBuffer_Type2 fileObjectBuffer_Type2;

    fileObjectBuffer_Type2.receive(in_stream);

    //fileObjectBuffer_Type2.log();

    RED_CHECK_EQUAL(sizeof(in_data) - 1, fileObjectBuffer_Type2.size());

    char out_data[sizeof(in_data)];

    OutStream out_stream(out_data, sizeof(out_data));

    fileObjectBuffer_Type2.emit(out_stream);
    //LOG(LOG_INFO, "out_stream_size=%u", (unsigned)out_stream.get_offset());
    //hexdump_av(out_stream.get_bytes());

    RED_CHECK_EQUAL(out_stream.get_offset(), in_stream.get_offset());
    RED_CHECK_EQUAL(0, memcmp(in_data, out_data, sizeof(in_data) - 1));
}

RED_AUTO_TEST_CASE(TestReparseGUIDDataBuffer)
{
//     const char in_data[] =
//
//         ""
//         ;
//     InStream in_stream(in_data, sizeof(in_data) - 1);
//
//     fscc::ReparseGUIDDataBuffer reparseGUIDDataBuffer;
//
//     reparseGUIDDataBuffer.receive(in_stream);
//
//     //file_both_directory_information.log();
//
//     RED_CHECK_EQUAL(sizeof(in_data) - 1, reparseGUIDDataBuffer.size());
//
//     char out_data[sizeof(in_data)];
//
//     OutStream out_stream(out_data, sizeof(out_data));
//
//     reparseGUIDDataBuffer.emit(out_stream);
//     //LOG(LOG_INFO, "out_stream_size=%u", (unsigned)out_stream.get_offset());
//     //hexdump_av(out_stream.get_bytes());
//
//     RED_CHECK_EQUAL(out_stream.get_offset(), in_stream.get_offset());
//     RED_CHECK_EQUAL(0, memcmp(in_data, out_data, sizeof(in_data) - 1));
}



RED_AUTO_TEST_CASE(TestFileAttributeTagInformation)
{
    const char in_data[] =
         "\x20\x00\x00\x00\x00\x00\x00\x00"  //.... .......
        ;
    InStream in_stream(in_data, sizeof(in_data) - 1);

    fscc::FileAttributeTagInformation fileAttributeTagInformation;

    fileAttributeTagInformation.receive(in_stream);

    //fileAttributeTagInformation.log();

    RED_CHECK_EQUAL(sizeof(in_data) - 1, fileAttributeTagInformation.size());

    char out_data[sizeof(in_data)];

    OutStream out_stream(out_data, sizeof(out_data));

    fileAttributeTagInformation.emit(out_stream);
    //LOG(LOG_INFO, "out_stream_size=%u", (unsigned)out_stream.get_offset());
    //hexdump_av(out_stream.get_bytes());

    RED_CHECK_EQUAL(out_stream.get_offset(), in_stream.get_offset());
    RED_CHECK_EQUAL(0, memcmp(in_data, out_data, sizeof(in_data) - 1));
}

RED_AUTO_TEST_CASE(TestFileBasicInformation)
{
    const char in_data[] =
                        "\x00\x02\x58\xcb\x31\x61\xd2\x01\x00\x76\x3e\x43" //$.....X.1a...v>C
        "\x86\x99\xd2\x01\x00\x02\x58\xcb\x31\x61\xd2\x01\x00\x20\xcf\x53" //......X.1a... .S
        "\xaa\x70\xd2\x01\x10\x00\x00\x00"                                 //.p......

        ;
    InStream in_stream(in_data, sizeof(in_data) - 1);

    fscc::FileBasicInformation fileBasicInformation;

    fileBasicInformation.receive(in_stream);

    //fileBasicInformation.log();

    RED_CHECK_EQUAL(sizeof(in_data) - 1, fileBasicInformation.size());

    char out_data[sizeof(in_data)];

    OutStream out_stream(out_data, sizeof(out_data));

    fileBasicInformation.emit(out_stream);
    //LOG(LOG_INFO, "out_stream_size=%u", (unsigned)out_stream.get_offset());
    //hexdump_av(out_stream.get_bytes());

    RED_CHECK_EQUAL(out_stream.get_offset(), in_stream.get_offset());
    RED_CHECK_EQUAL(0, memcmp(in_data, out_data, sizeof(in_data) - 1));
}

RED_AUTO_TEST_CASE(TestFileDirectoryInformation)
{
    const char in_data[] =
                        "\x00\x00\x00\x00\x00\x00\x00\x00\x7f\x6e\x7f\x7b" //k............n.{
        "\x88\x78\xd2\x01\x80\x36\x91\x6a\xe5\x98\xd2\x01\x80\x6e\x7f\x7b" //.x...6.j.....n.{
        "\x88\x78\xd2\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //.x..............
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00" //................
        "\x0e\x00\x00\x00\x73\x00\x68\x00\x61\x00\x72\x00\x65\x00\x2d\x00\x32\x00"     //.s.h.a.r.e.-.2.
        ;
    InStream in_stream(in_data, sizeof(in_data) - 1);

    fscc::FileDirectoryInformation fileDirectoryInformation;

    fileDirectoryInformation.receive(in_stream);

    //fileDirectoryInformation.log();

    //RED_CHECK_EQUAL(sizeof(in_data) - 1, fileDirectoryInformation.total_size());

    char out_data[sizeof(in_data)];

    OutStream out_stream(out_data, sizeof(out_data));

    fileDirectoryInformation.emit(out_stream);
    //LOG(LOG_INFO, "out_stream_size=%u", (unsigned)out_stream.get_offset());
    //hexdump_av(out_stream.get_bytes());

    RED_CHECK_EQUAL(out_stream.get_offset(), in_stream.get_offset());
    RED_CHECK_EQUAL(0, memcmp(in_data, out_data, sizeof(in_data) - 1));
}

RED_AUTO_TEST_CASE(TestFileDispositionInformation)
{
    const char in_data[] =
        "\x10" //.
        ;
    InStream in_stream(in_data, sizeof(in_data) - 1);

    fscc::FileDispositionInformation fileDispositionInformation;

    fileDispositionInformation.receive(in_stream);

    //fileDispositionInformation.log();

    RED_CHECK_EQUAL(sizeof(in_data) - 1, fileDispositionInformation.size());

    char out_data[sizeof(in_data)];

    OutStream out_stream(out_data, sizeof(out_data));

    fileDispositionInformation.emit(out_stream);
    //LOG(LOG_INFO, "out_stream_size=%u", (unsigned)out_stream.get_offset());
    //hexdump_av(out_stream.get_bytes());

    RED_CHECK_EQUAL(out_stream.get_offset(), in_stream.get_offset());
    RED_CHECK_EQUAL(0, memcmp(in_data, out_data, sizeof(in_data) - 1));
}

RED_AUTO_TEST_CASE(TestFileNamesInformation)
{
    const char in_data[] =
                                "\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x0e\x00\x00\x00\x73\x00\x68\x00\x61\x00\x72\x00\x65\x00\x2d\x00\x32\x00"     //.s.h.a.r.e.-.2.
        ;
    InStream in_stream(in_data, sizeof(in_data) - 1);

    fscc::FileNamesInformation fileNamesInformation;

    fileNamesInformation.receive(in_stream);

    //fileNamesInformation.log();

    //RED_CHECK_EQUAL(sizeof(in_data) - 1, fileNamesInformation.total_size());

    char out_data[sizeof(in_data)];

    OutStream out_stream(out_data, sizeof(out_data));

    fileNamesInformation.emit(out_stream);
    //LOG(LOG_INFO, "out_stream_size=%u", (unsigned)out_stream.get_offset());
    //hexdump_av(out_stream.get_bytes());

    RED_CHECK_EQUAL(out_stream.get_offset(), in_stream.get_offset());
    RED_CHECK_EQUAL(0, memcmp(in_data, out_data, sizeof(in_data) - 1));
}

RED_AUTO_TEST_CASE(TestFileStandardInformation)
{
    const char in_data[] =
         "\x00\x10\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00" //................
         "\x00\x00\x00\x00\x05\x00\x00\x00\x01\x01"         //..........
        ;

    InStream in_stream(in_data, sizeof(in_data) - 1);

    fscc::FileStandardInformation fileStandardInformation;

    fileStandardInformation.receive(in_stream);

    //fileStandardInformation.log();

    RED_CHECK_EQUAL(sizeof(in_data) - 1, fileStandardInformation.size());

    char out_data[sizeof(in_data)];

    OutStream out_stream(out_data, sizeof(out_data));

    fileStandardInformation.emit(out_stream);
    //LOG(LOG_INFO, "out_stream_size=%u", (unsigned)out_stream.get_offset());
    //hexdump_av(out_stream.get_bytes());

    RED_CHECK_EQUAL(out_stream.get_offset(), in_stream.get_offset());
    RED_CHECK_EQUAL(0, memcmp(in_data, out_data, sizeof(in_data) - 1));
}

RED_AUTO_TEST_CASE(TestFileFsSizeInformation)
{
    const char in_data[] =
         "\x00\x80\xad\x63\x50\x00\x00\x00"
         "\x00\x80\xad\x62\x50\x00\x00\x00\x08\x00\x00\x00\x00\x10\x00\x00"
        ;

    InStream in_stream(in_data, sizeof(in_data) - 1);

    fscc::FileFsSizeInformation fileFsSizeInformation;

    fileFsSizeInformation.receive(in_stream);

    //fileFsSizeInformation.log();

    RED_CHECK_EQUAL(sizeof(in_data) - 1, fileFsSizeInformation.size());

    char out_data[sizeof(in_data)];

    OutStream out_stream(out_data, sizeof(out_data));

    fileFsSizeInformation.emit(out_stream);
    //LOG(LOG_INFO, "out_stream_size=%u", (unsigned)out_stream.get_offset());
    //hexdump_av(out_stream.get_bytes());

    RED_CHECK_EQUAL(out_stream.get_offset(), in_stream.get_offset());
    RED_CHECK_EQUAL(0, memcmp(in_data, out_data, sizeof(in_data) - 1));
}

RED_AUTO_TEST_CASE(TestFileFsVolumeInformation)
{
    const char in_data[] =
         "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" //................
         "\x00\x00\x00\x00\x00"
        ;

    InStream in_stream(in_data, sizeof(in_data) - 1);

    fscc::FileFsVolumeInformation fileFsVolumeInformation;

    fileFsVolumeInformation.receive(in_stream);

    //fileFsVolumeInformation.log();

    RED_CHECK_EQUAL(sizeof(in_data) - 1, fileFsVolumeInformation.size());

    char out_data[sizeof(in_data)];

    OutStream out_stream(out_data, sizeof(out_data));

    fileFsVolumeInformation.emit(out_stream);
    //LOG(LOG_INFO, "out_stream_size=%u", (unsigned)out_stream.get_offset());
    //hexdump_av(out_stream.get_bytes());

    RED_CHECK_EQUAL(out_stream.get_offset(), in_stream.get_offset());
    RED_CHECK_EQUAL(0, memcmp(in_data, out_data, sizeof(in_data) - 1));
}
