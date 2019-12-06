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
    Author(s): Christophe Grosjean, Raphael Zhou, Clément Moroldo
*/

#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/sugar/array_view.hpp"
#include "utils/sugar/cast.hpp"
#include "core/RDP/clipboard.hpp"
#include "core/RDP/clipboard/format_list_serialize.hpp"


RED_AUTO_TEST_CASE(TestFormatDataResponsePDUEmitFileList)
{
    StaticOutStream<1024> ou_stream_fileList;

    const int cItems = 1;
    RDPECLIP::FormatDataResponsePDU_FileList fdr(cItems);
    fdr.emit(ou_stream_fileList);

    RED_CHECK_MEM(ou_stream_fileList.get_bytes(), "\x01\x00\x00\x00"_av);
}

RED_AUTO_TEST_CASE(TestFormatDataResponsePDURecvFileList)
{
    //  recv_fileList
    auto file_list_data =
        "\x01\x00\x00\x00\x64\x40\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x20\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x04\xb5\x9f\x37\xa0\xe2\xd1\x01\x00\x00\x00\x00"
        "\x11\x00\x00\x00\x61\x00\x62\x00\x63\x00\x64\x00\x65\x00\x2e\x00\x74\x00\x78\x00"
        "\x74\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00\x00\x00\x00\x00"_av;

    InStream in_stream_fileList_to_recv(file_list_data);
    RDPECLIP::FormatDataResponsePDU_FileList fdr_recv;
    fdr_recv.recv(in_stream_fileList_to_recv);

    RED_CHECK_EQUAL(fdr_recv.cItems, 1);
}

RED_AUTO_TEST_CASE(TestFormatDataResponsePDUEmitFilePic)
{
    // emit_metaFilePic
    StaticOutStream<1600> ou_stream_metaFilePic;

    int height=73;
    int width=220;
    int bpp=24;
    int data_lenght = height * width * 3;
    const double ARBITRARY_SCALE = 40;

    RDPECLIP::FormatDataResponsePDU_MetaFilePic fdr(data_lenght, width, height, bpp, ARBITRARY_SCALE);
    fdr.emit(ou_stream_metaFilePic);

    auto metafilepic_out_data = cstr_array_view(
        "\x08\x00\x00\x00\x60\x22\x00\x00"
        "\x68\x0b\x00\x00\x01\x00\x09\x00\x00\x03\x55\x5e\x00\x00\x00\x00"
        "\x3b\x5e\x00\x00\x00\x00\x04\x00\x00\x00\x03\x01\x08\x00\x05\x00"
        "\x00\x00\x0c\x02\xb7\xff\xdc\x00\x05\x00\x00\x00\x0b\x02\x00\x00"
        "\x00\x00\x3b\x5e\x00\x00\x41\x0b\x20\x00\xcc\x00\x49\x00\xdc\x00"
        "\x00\x00\x00\x00\xb7\xff\xdc\x00\x00\x00\x00\x00\x28\x00\x00\x00"
        "\xdc\x00\x00\x00\xb7\xff\xff\xff\x01\x00\x18\x00\x00\x00\x00\x00"
        "\x34\xbc\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x00\x00\x00\x00");

    RED_CHECK_MEM(metafilepic_out_data, ou_stream_metaFilePic.get_bytes());
}

RED_AUTO_TEST_CASE(TestFormatDataResponsePDURecvFilePic)
{
    // recv_metaFilePic

    unsigned height = 73;
    uint16_t negative_height16 = 0xFFFF - height + 1;
    uint32_t negative_height32 = 0xFFFFFFFF - height + 1;
    unsigned width = 220;
    unsigned bpp = 24;
    unsigned data_lenght = height * width * 3;
    const unsigned ARBITRARY_SCALE = 40;
    const auto metafilepic_out_data =
//         // header
//         "\x05\x00" // msgType  : 5 = CB_FORMAT_DATA_RESPONSE
//         "\x01\x00" // MsgFlags : 1 = CB_RESPONSE_OK
//         "\xb6\xbc\x00\x00" // dataLen : 48310

        // 2.2.5.2.1 Packed Metafile Payload (CLIPRDR_MFPICT)
        "\x08\x00\x00\x00" // mappingMode
        "\x60\x22\x00\x00"                              // xExt
        "\x68\x0b\x00\x00" // yExt

        // metaFileData
        "\x01\x00" // meta_header_type : 1 MEMORYMETAFILE
        "\x09\x00" // meta_header_size : 9 = 18 bytes
        "\x00\x03" // version 0x300 : WMF with DIBS

        "\x55\x5e\x00\x00" // 0X5e55 * 2 = 24149 * 2 = 48298 bytes

        "\x00\x00" // NumberOfObjects : 0 objects in metafile
        "\x3b\x5e\x00\x00" // MaxRecord : 0x5e3b = 24123 : 24123 * 2 = 48246 bytes

        "\x00\x00" // NumberOfMembers (not used)

        // Records
        "\x04\x00\x00\x00" // RecordSize 4 = 8 bytes
        "\x03\x01"         // 0x0103 : META_SETMAPMODE
        "\x08\x00"         // mappingMode: record specific placeholder

        "\x05\x00\x00\x00" // RecordSize 0x5 = 5 * 2 =  10
        "\x0c\x02"         // META_SETWINDOWEXT
        "\xb7\xff"         // height : -73 pixels (73 pixels upside down)
        "\xdc\x00"         // width :  220 pixels

        "\x05\x00\x00\x00" // RecordSize 0x5 = 5 * 2 =  10
        "\x0b\x02"         // META_SETWINDOWORG
        "\x00\x00"         // Origin y = 0
        "\x00\x00"         // Origin x = 0

        // META_DIBSTRETCHBLT This record specifies the transfer of a block of pixels in device-independent format according to a raster operation, with possible expansion or contraction.
        "\x3b\x5e\x00\x00" // RecordSize 0x5e3b = 24123 * 2 =  48246
        "\x41\x0b"         // META_DIBSTRETCHBLT
        "\x20\x00\xcc\x00" // rdParam (raster operation) : 0x00CC0020 : SRCCOPY
        "\x49\x00"         // SrcHeight : 73
        "\xdc\x00"         // SrcWidth : 220
        "\x00\x00"         // YSrc : 0
        "\x00\x00"         // XSrc : 0
        "\xb7\xff"         // DstHeight : -73
        "\xdc\x00"         // DstWidth : 220
        "\x00\x00"         // YDest 0
        "\x00\x00"         // XDest 0

        // DeviceIndependentBitmap  2.2.2.9 DeviceIndependentBitmap Object
            // BitMapInfoHeader
        "\x28\x00\x00\x00" // HeaderSize = 40 bytes
        "\xdc\x00\x00\x00" // Width : 220 pixels
        "\xb7\xff\xff\xff" // Height : -73 pixels
        "\x01\x00"         // Planes : 0x0001
        "\x18\x00"         // BitCount (depth) : 24 bpp
        "\x00\x00\x00\x00" // Compression : 0
        "\x34\xbc\x00\x00" // ImageSize : 48180 bytes
        "\x00\x00\x00\x00" // XPelsPerMeter : 0
        "\x00\x00\x00\x00" // YPelsPerMeter : 0
        "\x00\x00\x00\x00" // ColorUsed : 0
        "\x00\x00\x00\x00" // ColorImportant : 0
        ""_av
        ;

    InStream stream(metafilepic_out_data);

    RDPECLIP::FormatDataResponsePDU_MetaFilePic fdr;
    fdr.recv(stream);

    RED_CHECK_EQUAL(fdr.mappingMode, RDPECLIP::MM_ANISOTROPIC);
    RED_CHECK_EQUAL(fdr.xExt, width * ARBITRARY_SCALE);
    RED_CHECK_EQUAL(fdr.yExt, height * ARBITRARY_SCALE);

    MFF::MetaHeader & metaHeader = fdr.metaHeader;
    RED_CHECK_EQUAL(metaHeader.type, MFF::MEMORYMETAFILE);
    RED_CHECK_EQUAL(metaHeader.headerSize, 9);
    RED_CHECK_EQUAL(metaHeader.version, MFF::METAVERSION300);
    RED_CHECK_EQUAL(metaHeader.size, (data_lenght/2) + RDPECLIP::METAFILE_WORDS_HEADER_SIZE);
    RED_CHECK_EQUAL(metaHeader.numberOfObjects, 0);
    RED_CHECK_EQUAL(metaHeader.maxRecord, (data_lenght + RDPECLIP::META_DIBSTRETCHBLT_HEADER_SIZE)/2);
    RED_CHECK_EQUAL(metaHeader.numberOfMembers, 0);

    MFF::MetaSetWindowExt & metaSetWindowExt = fdr.metaSetWindowExt;
    RED_CHECK_EQUAL(metaSetWindowExt.height, negative_height16);
    RED_CHECK_EQUAL(metaSetWindowExt.width, width);

    MFF::MetaSetWindowOrg & metaSetWindowOrg = fdr.metaSetWindowOrg;
    RED_CHECK_EQUAL(metaSetWindowOrg.yOrg, 0);
    RED_CHECK_EQUAL(metaSetWindowOrg.xOrg, 0);

    MFF::MetaSetMapMod & metaSetMapMod = fdr.metaSetMapMod;
    RED_CHECK_EQUAL(metaSetMapMod.mappingMode, RDPECLIP::MM_ANISOTROPIC);

    MFF::DibStretchBLT & dibStretchBLT = fdr.dibStretchBLT;
    RED_CHECK_EQUAL(dibStretchBLT.recordSize, (data_lenght + RDPECLIP::META_DIBSTRETCHBLT_HEADER_SIZE)/2);
    RED_CHECK_EQUAL(dibStretchBLT.recordFunction, MFF::META_DIBSTRETCHBLT);
    RED_CHECK_EQUAL(dibStretchBLT.rasterOperation, RDPECLIP::SRCCOPY);
    RED_CHECK_EQUAL(dibStretchBLT.srcHeight, height);
    RED_CHECK_EQUAL(dibStretchBLT.srcWidth, width);
    RED_CHECK_EQUAL(dibStretchBLT.ySrc, 0);
    RED_CHECK_EQUAL(dibStretchBLT.xSrc, 0);
    RED_CHECK_EQUAL(dibStretchBLT.destHeight, negative_height16);
    RED_CHECK_EQUAL(dibStretchBLT.destWidth, width);
    RED_CHECK_EQUAL(dibStretchBLT.yDest, 0);
    RED_CHECK_EQUAL(dibStretchBLT.xDest, 0);

    MFF::DibStretchBLT::BitmapInfoHeader & bitmapInfoHeader = dibStretchBLT.bitmapInfoHeader;
    RED_CHECK_EQUAL(bitmapInfoHeader.headerSize, 40);
    RED_CHECK_EQUAL(bitmapInfoHeader.height, negative_height32);
    RED_CHECK_EQUAL(bitmapInfoHeader.width, width);
    RED_CHECK_EQUAL(bitmapInfoHeader.planes, 0x0001);
    RED_CHECK_EQUAL(bitmapInfoHeader.bitCount, bpp);
    RED_CHECK_EQUAL(bitmapInfoHeader.compression, 0);
    RED_CHECK_EQUAL(bitmapInfoHeader.imageSize, data_lenght);
    RED_CHECK_EQUAL(bitmapInfoHeader.xPelsPerMeter, 0);
    RED_CHECK_EQUAL(bitmapInfoHeader.yPelsPerMeter, 0);
    RED_CHECK_EQUAL(bitmapInfoHeader.colorUsed, 0);
    RED_CHECK_EQUAL(bitmapInfoHeader.colorImportant, 0);
}


RED_AUTO_TEST_CASE(TestMetaFilePicDescriptor)
{
   const auto metafilepic_in_data =
       // header
            "\x05\x00" // msgType  : 5 = CB_FORMAT_DATA_RESPONSE
            "\x01\x00" // MsgFlags : 1 = CB_RESPONSE_OK
            "\xb6\xbc\x00\x00" // dataLen : 48310

            // 2.2.5.2.1 Packed Metafile Payload (CLIPRDR_MFPICT)
            "\x08\x00\x00\x00" // mappingMode
            "\x60\x22\x00\x00"                              // xExt
            "\x68\x0b\x00\x00" // yExt

            // metaFileData
            "\x01\x00" // meta_header_type : 1 MEMORYMETAFILE
            "\x09\x00" // meta_header_size : 9 = 18 bytes
            "\x00\x03" // version 0x300 : WMF with DIBS

            "\x55\x5e\x00\x00" // 0X5e55 * 2 = 24149 * 2 = 48298 bytes

            "\x00\x00" // NumberOfObjects : 0 objects in metafile
            "\x3b\x5e\x00\x00" // MaxRecord : 0x5e3b = 24123 : 24123 * 2 = 48246 bytes

            "\x00\x00" // NumberOfMembers (not used)

            // Records
            "\x04\x00\x00\x00" // RecordSize 4 = 8 bytes
            "\x03\x01"         // 0x0103 : META_SETMAPMODE
            "\x08\x00"         // rdParam : record specific placeholder

            "\x05\x00\x00\x00" // RecordSize 0x5 = 5 * 2 =  10
            "\x0c\x02"         // META_SETWINDOWEXT
            "\xb7\xff"         // height : -73 pixels (73 pixels upside down)
            "\xdc\x00"         // width :  220 pixels

            "\x05\x00\x00\x00" // RecordSize 0x5 = 5 * 2 =  10
            "\x0b\x02"         // META_SETWINDOWORG
            "\x00\x00"         // Origin y = 0
            "\x00\x00"         // Origin x = 0

            // META_DIBSTRETCHBLT This record specifies the transfer of a block of pixels in device-independent format according to a raster operation, with possible expansion or contraction.
            "\x3b\x5e\x00\x00" // RecordSize 0x5e3b = 24123 * 2 =  48246
            "\x41\x0b"         // META_DIBSTRETCHBLT
            "\x20\x00\xcc\x00" // rdParam (raster operation) : 0x00CC0020 : SRCCOPY
            "\x49\x00"         // SrcHeight : 73
            "\xdc\x00"         // SrcWidth : 220
            "\x00\x00"         // YSrc : 0
            "\x00\x00"         // XSrc : 0
            "\xb7\xff"         // DstHeight : -73
            "\xdc\x00"         // DstWidth : 220
            "\x00\x00"         // YDest 0
            "\x00\x00"         // XDest 0

            // DeviceIndependentBitmap  2.2.2.9 DeviceIndependentBitmap Object
                // BitMapInfoHeader
            "\x28\x00\x00\x00" // HeaderSize = 40 bytes
            "\xdc\x00\x00\x00" // Width : 220 pixels
            "\xb7\xff\xff\xff" // Height : -73 pixels
            "\x01\x00"         // Planes : 0x0001
            "\x18\x00"         // BitCount (depth) : 24 bpp
            "\x00\x00\x00\x00" // Compression : 0
            "\x34\xbc\x00\x00" // ImageSize : 48180 bytes
            "\x00\x00\x00\x00" // XPelsPerMeter : 0
            "\x00\x00\x00\x00" // YPelsPerMeter : 0
            "\x00\x00\x00\x00" // ColorUsed : 0
            "\x00\x00\x00\x00" // ColorImportant : 0
            ""_av
            ;

   InStream in_stream_metaFilePic(metafilepic_in_data);
   RDPECLIP::MetaFilePicDescriptor mfpd;
   mfpd.receive(in_stream_metaFilePic);

   RED_CHECK_EQUAL(mfpd.recordSize, 48246);
   RED_CHECK_EQUAL(mfpd.height    , 73);
   RED_CHECK_EQUAL(mfpd.width     , 220);
   RED_CHECK_EQUAL(mfpd.bpp       , 24);
   RED_CHECK_EQUAL(mfpd.imageSize , 48180);
}


RED_AUTO_TEST_CASE(TestFileDescriptor)
{
    auto in_data =
            "\x64\x40\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // d@..............
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
            "\x00\x00\x00\x00\x20\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // .... ...........
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x1e\x9b\x65\x20\xb2\xc6\x01" // ...........e ...
            "\x00\x00\x00\x00\x5d\x1b\x00\x00\x45\x00\x75\x00\x6c\x00\x61\x00" // ....]...E.u.l.a.
            "\x2e\x00\x74\x00\x78\x00\x74\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ..t.x.t.........
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" // ................
            ""_av
        ;
    InStream in_stream(in_data);

    RDPECLIP::FileDescriptor file_descriptor;

    file_descriptor.receive(in_stream);

    //file_descriptor.log(LOG_INFO);

    RED_CHECK_EQUAL(in_data.size(), file_descriptor.size());

    StaticOutStream<1200> out_stream;

    file_descriptor.emit(out_stream);

    RED_CHECK_MEM(out_stream.get_bytes(), in_data);
}


RED_AUTO_TEST_CASE(TestFileContentsRequestPDURangeEmit)
{
    // inData
    uint32_t ID = 1;
    uint32_t index = 3;
    uint32_t flag = RDPECLIP::FILECONTENTS_RANGE;
    uint32_t size_low = 0x00000007;
    uint32_t size_high = 0x00000000;

    // Init Stream
    StaticOutStream<64> out_stream;
    RDPECLIP::FileContentsRequestPDU fileContentsRequest(ID, flag, index, size_low, size_high, true);
    fileContentsRequest.emit(out_stream);

    auto exp_data = cstr_array_view(
        "\x01\x00\x00\x00\x02\x00\x00\x00\x03\x00\x00\x00\x07\x00\x00\x00"
        "\x00\x00\x00\x00\x01\x00\x00\x00");

    RED_CHECK_MEM(exp_data, out_stream.get_bytes());
}

RED_AUTO_TEST_CASE(TestFileContentsRequestPDURangeRecv)
{
    // inData
    uint32_t ID = 1;
    uint32_t index = 3;
    uint32_t flag = RDPECLIP::FILECONTENTS_RANGE;
    uint64_t size = 0x0000000000000007;

    auto data =
        "\x01\x00\x00\x00\x03\x00\x00\x00\x02\x00\x00\x00\x07\x00\x00\x00"
        "\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00"_av;

    InStream stream(data);
    RDPECLIP::FileContentsRequestPDU fileContentsRequest;
    fileContentsRequest.receive(stream);

    RED_CHECK_EQUAL(fileContentsRequest.streamId(), ID);
    RED_CHECK_EQUAL(fileContentsRequest.dwFlags() , flag);
    RED_CHECK_EQUAL(fileContentsRequest.lindex() , index);
    RED_CHECK_EQUAL(fileContentsRequest.position(), size);
}

RED_AUTO_TEST_CASE(TestFileContentsRequestPDUSizeEmit)
{
    // inData
    uint32_t ID = 1;
    uint32_t index = 3;
    uint32_t flag = RDPECLIP::FILECONTENTS_SIZE;
    uint64_t size = 0x0000000000000007;

    // Init Stream
    StaticOutStream<64> out_stream;
    RDPECLIP::FileContentsRequestPDU fileContentsRequest(ID, flag, index, size, size, true);
    fileContentsRequest.emit(out_stream);

    auto exp_data = cstr_array_view(
        "\x01\x00\x00\x00\x01\x00\x00\x00\x03\x00\x00\x00\x07\x00\x00\x00"
        "\x07\x00\x00\x00\x01\x00\x00\x00");

    RED_CHECK_MEM(exp_data, out_stream.get_bytes());
}

RED_AUTO_TEST_CASE(TestFileContentsRequestPDUSizeRecv)
{
    // inData
    uint32_t ID = 1;
    uint32_t index = 3;
    uint32_t flag = RDPECLIP::FILECONTENTS_SIZE;
    uint64_t size = 0x0000000000000007;

    auto data =
        "\x01\x00\x00\x00\x03\x00\x00\x00"
        "\x01\x00\x00\x00\x07\x00\x00\x00\x00\x00\x00\x00\x08\x00\x00\x00"
        "\x01\x00\x00\x00"_av;

    InStream stream(data);
    RDPECLIP::FileContentsRequestPDU fileContentsRequest;
    fileContentsRequest.receive(stream);

    RED_CHECK_EQUAL(fileContentsRequest.streamId(), ID);
    RED_CHECK_EQUAL(fileContentsRequest.dwFlags() , flag);
    RED_CHECK_EQUAL(fileContentsRequest.lindex()  , index);
    RED_CHECK_EQUAL(fileContentsRequest.position(), size);
}

RED_AUTO_TEST_CASE(TestFormatList_extract_serialize)
{
    struct Data{
        char const* name;
        Cliprdr::IsLongFormat is_long_format;
        Cliprdr::IsAscii is_ascii;
        array_view_const_char expected_result;
        std::vector<Cliprdr::FormatNameRef> formats;
        std::vector<Cliprdr::FormatNameRef> formats_ref {};
    };

    RED_TEST_CONTEXT_DATA(Data const& data,
        "TestName: " << data.name <<
        "  UseLongFormat: " << bool(data.is_long_format) <<
        "  IsAscii: " << bool(data.is_ascii), {
        Data{"text1",
            Cliprdr::IsLongFormat(true),
            Cliprdr::IsAscii(false),
            "\x02\x00\x00\x00\x06\x00\x00\x00"
            "\x01\x00\x00\x00\x00\x00"_av, std::vector{
                Cliprdr::FormatNameRef{RDPECLIP::CF_TEXT, {}},
            }},
        Data{"text + unicode",
            Cliprdr::IsLongFormat(true),
            Cliprdr::IsAscii(false),
            "\x02\x00\x00\x00\x0c\x00\x00\x00"
            "\x01\x00\x00\x00\x00\x00"
            "\x0d\x00\x00\x00\x00\x00"_av, std::vector{
                Cliprdr::FormatNameRef{RDPECLIP::CF_TEXT, {}},
                Cliprdr::FormatNameRef{RDPECLIP::CF_UNICODETEXT, {}}
            }},
        Data{"5 formats",
            Cliprdr::IsLongFormat(true),
            Cliprdr::IsAscii(false),
            "\x02\x00\x00\x00\x5e\x00\x00\x00\x9a\xbb\x00\x00\x46\x00\x69\x00" //....^.......F.i.
            "\x6c\x00\x65\x00\x43\x00\x6f\x00\x6e\x00\x74\x00\x65\x00\x6e\x00" //l.e.C.o.n.t.e.n.
            "\x74\x00\x73\x00\x00\x00\x99\xbb\x00\x00\x46\x00\x69\x00\x6c\x00" //t.s.......F.i.l.
            "\x65\x00\x47\x00\x72\x00\x6f\x00\x75\x00\x70\x00\x44\x00\x65\x00" //e.G.r.o.u.p.D.e.
            "\x73\x00\x63\x00\x72\x00\x69\x00\x70\x00\x74\x00\x6f\x00\x72\x00" //s.c.r.i.p.t.o.r.
            "\x57\x00\x00\x00\x0d\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00" //W...............
            "\x03\x00\x00\x00\x00\x00"_av, std::vector{
                Cliprdr::FormatNameRef{48026, "FileContents"_av},
                Cliprdr::FormatNameRef{48025, "FileGroupDescriptorW"_av},
                Cliprdr::FormatNameRef{RDPECLIP::CF_UNICODETEXT, {}},
                Cliprdr::FormatNameRef{RDPECLIP::CF_TEXT, {}},
                Cliprdr::FormatNameRef{RDPECLIP::CF_METAFILEPICT, {}},
            }},
        Data{"user format 1",
            Cliprdr::IsLongFormat(true),
            Cliprdr::IsAscii(false),
            "\x02\x00\x00\x00\x0e\x00\x00\x00"
            "\x00\x7D\x00\x00T\x00" "e\x00s\x00t\x00\x00\x00"_av, std::vector{
                Cliprdr::FormatNameRef{32000, "Test"_av}
            }},
        Data{"user format 2",
            Cliprdr::IsLongFormat(true),
            Cliprdr::IsAscii(false),
            "\x02\x00\x00\x00\x20\x00\x00\x00"
            "\x00\x7D\x00\x00T\x00" "e\x00s\x00t\x00""1\x00\x00\x00"
            "\x01\x7D\x00\x00T\x00" "e\x00s\x00t\x00""2\x00\x00\x00"_av, std::vector{
                Cliprdr::FormatNameRef{32000, "Test1"_av},
                Cliprdr::FormatNameRef{32001, "Test2"_av}
            }},

        Data{"test 5",
            Cliprdr::IsLongFormat(false),
            Cliprdr::IsAscii(false),
            "\x02\x00\x00\x00\x24\x00\x00\x00"
            "\x00\x7D\x00\x00"
               "T\x00" "e\x00" "s\x00" "t\x00\x00\x00\x00\x00\x00\x00\x00\x00"
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"_av, std::vector{
                Cliprdr::FormatNameRef{32000, "Test"_av},
            }},
        Data{"test 6",
            Cliprdr::IsLongFormat(false),
            Cliprdr::IsAscii(false),
            "\x02\x00\x00\x00\x48\x00\x00\x00"
            "\x00\x7D\x00\x00"
               "T\x00" "e\x00" "s\x00" "t\x00" "1\x00\x00\x00\x00\x00\x00\x00"
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
            "\x01\x7D\x00\x00"
               "T\x00" "e\x00" "s\x00" "t\x00" "2\x00\x00\x00\x00\x00\x00\x00"
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"_av, std::vector{
                Cliprdr::FormatNameRef{32000, "Test1"_av},
                Cliprdr::FormatNameRef{32001, "Test2"_av},
            }},
        Data{"test 7",
            Cliprdr::IsLongFormat(false),
            Cliprdr::IsAscii(true),
            "\x02\x00\x04\x00\x24\x00\x00\x00"
            "\x00\x7D\x00\x00"
            "RedemptionClipboard"
            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"_av, std::vector{
                Cliprdr::FormatNameRef{32000, "RedemptionClipboard"_av},
            }},
        Data{"test 8",
            Cliprdr::IsLongFormat(false),
            Cliprdr::IsAscii(false),
            "\x02\x00\x00\x00\x24\x00\x00\x00"
            "\x00\x7D\x00\x00"
            "R\x00" "\xe9\x00" "d\x00" "e\x00" "m\x00" "p\x00" "t\x00" "i\x00"
            "o\x00" "n\x00" "C\x00" "l\x00" "i\x00" "p\x00" "b\x00" "\x00\x00"_av, std::vector{
                Cliprdr::FormatNameRef{32000, "RédemptionClipboard"_av},
            }, std::vector{
                Cliprdr::FormatNameRef{32000, "RédemptionClipb"_av},
            }},
        Data{"test 9",
            Cliprdr::IsLongFormat(false),
            Cliprdr::IsAscii(true),
            "\x02\x00\x04\x00\x24\x00\x00\x00"
            "\x00\x7D\x00\x00"
            "0123456789012345678901234567890\x00"_av, std::vector{
                Cliprdr::FormatNameRef{32000, "0123456789012345678901234567890123456789"_av},
            }, std::vector{
                Cliprdr::FormatNameRef{32000, "0123456789012345678901234567890"_av},
            }},
    }) {
        StaticOutStream<512> out_stream;
        Cliprdr::format_list_serialize_with_header(
            out_stream, data.is_long_format, data.formats);

        InStream in_stream(out_stream.get_bytes());
        RDPECLIP::CliprdrHeader header;
        header.recv(in_stream);
        RED_CHECK(bool(header.msgFlags() & RDPECLIP::CB_ASCII_NAMES) == bool(data.is_ascii));

        RED_CHECK_MEM(data.expected_result, out_stream.get_bytes());

        auto format_ref = data.formats_ref.empty() ? data.formats : data.formats_ref;
        auto it = format_ref.begin();
        Cliprdr::format_list_extract(in_stream, data.is_long_format, data.is_ascii,
            [&](uint32_t format_id, auto name){
                RED_TEST_CONTEXT("idx format: " << (it-format_ref.begin())) {
                    RED_REQUIRE((it != format_ref.end()));
                    RED_CHECK(format_id == it->format_id());
                    Cliprdr::FormatName format_name(0, name);
                    RED_CHECK_SMEM(format_name.utf8_name(), it->utf8_name());
                    ++it;
                }
            });
        RED_CHECK((it == format_ref.end()));
    }
}
