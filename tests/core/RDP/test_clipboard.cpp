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

#define RED_TEST_MODULE TestGCC
#include "test_only/test_framework/redemption_unit_tests.hpp"

#include "utils/sugar/array_view.hpp"
#include "utils/sugar/cast.hpp"
#include "core/RDP/clipboard.hpp"


RED_AUTO_TEST_CASE(TestFormatDataResponsePDUEmitFileList)
{
    StaticOutStream<1024> ou_stream_fileList;

    const int cItems = 1;
    RDPECLIP::FormatDataResponsePDU_FileList fdr(cItems);
    fdr.emit(ou_stream_fileList);

    RED_CHECK_MEM_C(ou_stream_fileList.get_bytes(), "\x01\x00\x00\x00");
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

    int height=73;
    uint16_t negative_height16 = 0xFFFF - height + 1;
    uint32_t negative_height32 = 0xFFFFFFFF - height + 1;
    int width=220;
    int bpp=24;
    int data_lenght = height * width * 3;
    const double ARBITRARY_SCALE = 40;
    const char metafilepic_out_data[] =
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
        ;

    InStream stream(metafilepic_out_data, 124);

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
   const char metafilepic_in_data[] =
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
            ;

   InStream in_stream_metaFilePic(metafilepic_in_data, 132);
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
    const char in_data[] =
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
        ;
    InStream in_stream(in_data, sizeof(in_data) - 1);

    RDPECLIP::FileDescriptor file_descriptor;

    file_descriptor.receive(in_stream);

    //file_descriptor.log(LOG_INFO);

    RED_CHECK_EQUAL(sizeof(in_data) - 1, file_descriptor.size());

    char out_data[sizeof(in_data)];

    OutStream out_stream(out_data, sizeof(out_data));

    file_descriptor.emit(out_stream);
//     LOG(LOG_INFO, "out_stream_size=%u", (unsigned)out_stream.get_offset());
//     hexdump_av(out_stream.get_bytes());

    RED_CHECK_EQUAL(out_stream.get_offset(), in_stream.get_offset());
    RED_CHECK_EQUAL(0, memcmp(in_data, out_data, sizeof(in_data) - 1));
}



RED_AUTO_TEST_CASE(TestFormatListPDU)
{
    RDPECLIP::FormatListPDUEx format_list_pdu;
    format_list_pdu.add_format_name(48026, "FileContents");
    format_list_pdu.add_format_name(48025, "FileGroupDescriptorW");
    format_list_pdu.add_format_name(RDPECLIP::CF_UNICODETEXT);
    format_list_pdu.add_format_name(RDPECLIP::CF_TEXT);
    format_list_pdu.add_format_name(RDPECLIP::CF_METAFILEPICT);

    const bool use_long_format_names = true;
    const bool in_ASCII_8 = format_list_pdu.will_be_sent_in_ASCII_8(use_long_format_names);

    RDPECLIP::CliprdrHeader clipboard_header(RDPECLIP::CB_FORMAT_LIST,
        RDPECLIP::CB_RESPONSE__NONE_ | (in_ASCII_8 ? RDPECLIP::CB_ASCII_NAMES : 0),
        format_list_pdu.size(use_long_format_names));

    // Init stream format list PDU long name
    StaticOutStream<1024> out_stream;

    clipboard_header.emit(out_stream);
    format_list_pdu.emit(out_stream, use_long_format_names);

    const char exp_data[] =
        "\x02\x00\x00\x00\x5e\x00\x00\x00\x9a\xbb\x00\x00\x46\x00\x69\x00" //....^.......F.i.
        "\x6c\x00\x65\x00\x43\x00\x6f\x00\x6e\x00\x74\x00\x65\x00\x6e\x00" //l.e.C.o.n.t.e.n.
        "\x74\x00\x73\x00\x00\x00\x99\xbb\x00\x00\x46\x00\x69\x00\x6c\x00" //t.s.......F.i.l.
        "\x65\x00\x47\x00\x72\x00\x6f\x00\x75\x00\x70\x00\x44\x00\x65\x00" //e.G.r.o.u.p.D.e.
        "\x73\x00\x63\x00\x72\x00\x69\x00\x70\x00\x74\x00\x6f\x00\x72\x00" //s.c.r.i.p.t.o.r.
        "\x57\x00\x00\x00\x0d\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00" //W...............
        "\x03\x00\x00\x00\x00\x00";

    RED_CHECK_EQUAL(out_stream.get_offset(), sizeof(exp_data) - 1);
    RED_CHECK_EQUAL(::memcmp(out_stream.get_data(), exp_data, std::min(out_stream.get_offset(), sizeof(exp_data) - 1)), 0);
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

    const char data[] =
        "\x01\x00\x00\x00\x03\x00\x00\x00\x02\x00\x00\x00\x07\x00\x00\x00"
        "\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00";

    InStream stream(data, sizeof(data)-1);
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

    const char data[] =
        "\x01\x00\x00\x00\x03\x00\x00\x00"
        "\x01\x00\x00\x00\x07\x00\x00\x00\x00\x00\x00\x00\x08\x00\x00\x00"
        "\x01\x00\x00\x00";

    InStream stream(data, sizeof(data)-1);
    RDPECLIP::FileContentsRequestPDU fileContentsRequest;
    fileContentsRequest.receive(stream);

    RED_CHECK_EQUAL(fileContentsRequest.streamId(), ID);
    RED_CHECK_EQUAL(fileContentsRequest.dwFlags() , flag);
    RED_CHECK_EQUAL(fileContentsRequest.lindex()  , index);
    RED_CHECK_EQUAL(fileContentsRequest.position(), size);
}

RED_AUTO_TEST_CASE(TestFormatListPDUEx_Emit_LongFormatName)
{
    const bool use_long_format_names = true;

    {
        RDPECLIP::FormatListPDUEx format_list_pdu;

        format_list_pdu.add_format_name(RDPECLIP::CF_TEXT, "");

        RED_CHECK_EQUAL(6, format_list_pdu.size(use_long_format_names));

        RED_CHECK(!format_list_pdu.will_be_sent_in_ASCII_8(use_long_format_names));

        StaticOutStream<512> out_stream;

        format_list_pdu.emit(out_stream, use_long_format_names);

        auto exp_data = cstr_array_view("\x01\x00\x00\x00\x00\x00");

        RED_CHECK_EQUAL(6, out_stream.get_offset());
        RED_CHECK_MEM(exp_data, out_stream.get_bytes());
    }

    {
        RDPECLIP::FormatListPDUEx format_list_pdu;

        format_list_pdu.add_format_name(RDPECLIP::CF_TEXT, "");
        format_list_pdu.add_format_name(RDPECLIP::CF_UNICODETEXT, "");

        RED_CHECK_EQUAL(12, format_list_pdu.size(use_long_format_names));

        RED_CHECK(!format_list_pdu.will_be_sent_in_ASCII_8(use_long_format_names));

        StaticOutStream<512> out_stream;

        format_list_pdu.emit(out_stream, use_long_format_names);

        auto exp_data = cstr_array_view(
                "\x01\x00\x00\x00\x00\x00"
                "\x0d\x00\x00\x00\x00\x00"
            );

        RED_CHECK_EQUAL(12, out_stream.get_offset());
        RED_CHECK_MEM(exp_data, out_stream.get_bytes());
    }

    {
        RDPECLIP::FormatListPDUEx format_list_pdu;

        format_list_pdu.add_format_name(32000, "Test");

        RED_CHECK_EQUAL(14, format_list_pdu.size(use_long_format_names));

        RED_CHECK(!format_list_pdu.will_be_sent_in_ASCII_8(use_long_format_names));

        StaticOutStream<512> out_stream;

        format_list_pdu.emit(out_stream, use_long_format_names);

        auto exp_data = cstr_array_view("\x00\x7D\x00\x00T\x00" "e\x00s\x00t\x00\x00\x00");

        RED_CHECK_EQUAL(14, out_stream.get_offset());
        RED_CHECK_MEM(exp_data, out_stream.get_bytes());
    }
}

RED_AUTO_TEST_CASE(TestFormatListPDUEx_Recv_LongFormatName)
{
    const bool use_long_format_names = true;

    {
        auto data = cstr_array_view("\x01\x00\x00\x00\x00\x00");

        InStream in_stream(data);

        RDPECLIP::FormatListPDUEx format_list_pdu;

        const bool in_ASCII_8 = false;
        format_list_pdu.recv(in_stream, use_long_format_names, in_ASCII_8);

        RED_CHECK_EQUAL(6, format_list_pdu.size(use_long_format_names));

        RED_CHECK(!format_list_pdu.will_be_sent_in_ASCII_8(use_long_format_names));

        auto format_name_it = format_list_pdu.begin();
        auto format_name_end = format_list_pdu.end();

        RED_CHECK_EQUAL(1, format_name_end - format_name_it);
        RED_CHECK_EQUAL(1, format_name_it->formatId());
        RED_CHECK_EQUAL("", format_name_it->format_name());
    }

    {
        auto data = cstr_array_view(
                "\x01\x00\x00\x00\x00\x00"
                "\x0d\x00\x00\x00\x00\x00"
            );

        InStream in_stream(data);

        RDPECLIP::FormatListPDUEx format_list_pdu;

        const bool in_ASCII_8 = false;
        format_list_pdu.recv(in_stream, use_long_format_names, in_ASCII_8);

        RED_CHECK_EQUAL(12, format_list_pdu.size(use_long_format_names));

        RED_CHECK(!format_list_pdu.will_be_sent_in_ASCII_8(use_long_format_names));

        auto format_name_it = format_list_pdu.begin();
        auto format_name_end = format_list_pdu.end();

        RED_CHECK_EQUAL(2, format_name_end - format_name_it);

        RED_CHECK_EQUAL(RDPECLIP::CF_TEXT, format_name_it[0].formatId());
        RED_CHECK_EQUAL("", format_name_it[0].format_name());

        RED_CHECK_EQUAL(RDPECLIP::CF_UNICODETEXT, format_name_it[1].formatId());
        RED_CHECK_EQUAL("", format_name_it[1].format_name());
    }

    {
        auto data = cstr_array_view("\x01\x00\x00\x00" "T\x00" "e\x00" "s\x00" "t\x00" "\x00\x00");

        InStream in_stream(data);

        RDPECLIP::FormatListPDUEx format_list_pdu;

        const bool in_ASCII_8 = false;
        format_list_pdu.recv(in_stream, use_long_format_names, in_ASCII_8);

        RED_CHECK_EQUAL(14, format_list_pdu.size(use_long_format_names));

        RED_CHECK(!format_list_pdu.will_be_sent_in_ASCII_8(use_long_format_names));

        auto format_name_it = format_list_pdu.begin();
        auto format_name_end = format_list_pdu.end();

        RED_CHECK_EQUAL(1, format_name_end - format_name_it);
        RED_CHECK_EQUAL(1, format_name_it->formatId());
        RED_CHECK_EQUAL("Test", format_name_it->format_name());
    }

    {
        auto data = cstr_array_view(
                "\x01\x00\x00\x00" "T\x00" "e\x00" "s\x00" "t\x00" "1\x00" "\x00\x00"
                "\x02\x00\x00\x00" "T\x00" "e\x00" "s\x00" "t\x00" "2\x00" "\x00\x00"
            );

        InStream in_stream(data);

        RDPECLIP::FormatListPDUEx format_list_pdu;

        const bool in_ASCII_8 = false;
        format_list_pdu.recv(in_stream, use_long_format_names, in_ASCII_8);

        RED_CHECK_EQUAL(32, format_list_pdu.size(use_long_format_names));

        RED_CHECK(!format_list_pdu.will_be_sent_in_ASCII_8(use_long_format_names));

        auto format_name_it = format_list_pdu.begin();
        auto format_name_end = format_list_pdu.end();

        RED_CHECK_EQUAL(2, format_name_end - format_name_it);

        RED_CHECK_EQUAL(1, format_name_it[0].formatId());
        RED_CHECK_EQUAL("Test1", format_name_it[0].format_name());

        RED_CHECK_EQUAL(2, format_name_it[1].formatId());
        RED_CHECK_EQUAL("Test2", format_name_it[1].format_name());
    }
}

RED_AUTO_TEST_CASE(TestFormatListPDUEx_Emit_ShortFormatName)
{
    const bool use_long_format_names_false = false;

    {
        RDPECLIP::FormatListPDUEx format_list_pdu;

        format_list_pdu.add_format_name(32000, "Test");

        RED_CHECK_EQUAL(36, format_list_pdu.size(use_long_format_names_false));

        RED_CHECK(!format_list_pdu.will_be_sent_in_ASCII_8(use_long_format_names_false));

        StaticOutStream<512> out_stream;

        format_list_pdu.emit(out_stream, use_long_format_names_false);

        auto exp_data = cstr_array_view(
                "\x00\x7D\x00\x00"
                "T\x00" "e\x00" "s\x00" "t\x00"
                                                "\x00\x00\x00\x00\x00\x00\x00\x00"
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
            );

        RED_CHECK_EQUAL(36, out_stream.get_offset());
        RED_CHECK_MEM(exp_data, out_stream.get_bytes());
    }

    {
        RDPECLIP::FormatListPDUEx format_list_pdu;

        format_list_pdu.add_format_name(32000, "Test1");
        format_list_pdu.add_format_name(32001, "Test2");

        RED_CHECK_EQUAL(72, format_list_pdu.size(use_long_format_names_false));

        RED_CHECK(!format_list_pdu.will_be_sent_in_ASCII_8(use_long_format_names_false));

        StaticOutStream<512> out_stream;

        format_list_pdu.emit(out_stream, use_long_format_names_false);

        auto exp_data = cstr_array_view(
                "\x00\x7D\x00\x00"
                "T\x00" "e\x00" "s\x00" "t\x00" "1\x00"
                                                        "\x00\x00\x00\x00\x00\x00"
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"

                "\x01\x7D\x00\x00"
                "T\x00" "e\x00" "s\x00" "t\x00" "2\x00"
                                                        "\x00\x00\x00\x00\x00\x00"
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
            );

        RED_CHECK_EQUAL(72, out_stream.get_offset());
        RED_CHECK_MEM(exp_data, out_stream.get_bytes());
    }

    {
        RDPECLIP::FormatListPDUEx format_list_pdu;

        format_list_pdu.add_format_name(32000, "RedemptionClipboard");

        RED_CHECK_EQUAL(36, format_list_pdu.size(use_long_format_names_false));

        RED_CHECK(format_list_pdu.will_be_sent_in_ASCII_8(use_long_format_names_false));

        StaticOutStream<512> out_stream;

        format_list_pdu.emit(out_stream, use_long_format_names_false);

        auto exp_data = cstr_array_view(
                "\x00\x7D\x00\x00"
                "RedemptionClipboard"
                            "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
            );

        RED_CHECK_EQUAL(36, out_stream.get_offset());
        RED_CHECK_MEM(exp_data, out_stream.get_bytes());
    }

    {
        RDPECLIP::FormatListPDUEx format_list_pdu;

        format_list_pdu.add_format_name(32000, "RédemptionClipboard");

        RED_CHECK_EQUAL(36, format_list_pdu.size(use_long_format_names_false));

        RED_CHECK(!format_list_pdu.will_be_sent_in_ASCII_8(use_long_format_names_false));

        StaticOutStream<512> out_stream;

        format_list_pdu.emit(out_stream, use_long_format_names_false);

        auto exp_data = cstr_array_view(
                "\x00\x7D\x00\x00"
                "R\x00"
                        "\xe9\x00"  // 'é'
                                "d\x00" "e\x00" "m\x00" "p\x00" "t\x00" "i\x00"
                "o\x00" "n\x00" "C\x00" "l\x00" "i\x00" "p\x00" "b\x00"
                                                                        "\x00\x00"
            );

        RED_CHECK_EQUAL(36, out_stream.get_offset());
        RED_CHECK_MEM(exp_data, out_stream.get_bytes());
    }

    {
        RDPECLIP::FormatListPDUEx format_list_pdu;

        format_list_pdu.add_format_name(32000, "0123456789012345678901234567890123456789");

        RED_CHECK_EQUAL(36, format_list_pdu.size(use_long_format_names_false));

        RED_CHECK(format_list_pdu.will_be_sent_in_ASCII_8(use_long_format_names_false));

        StaticOutStream<512> out_stream;

        format_list_pdu.emit(out_stream, use_long_format_names_false);

        auto exp_data = cstr_array_view(
                "\x00\x7D\x00\x00"
                "0123456789012345678901234567890"
                                                                            "\x00"
            );

        RED_CHECK_EQUAL(36, out_stream.get_offset());
        RED_CHECK_MEM(exp_data, out_stream.get_bytes());
    }
}

RED_AUTO_TEST_CASE(TestFormatListPDUEx_Recv_ShortFormatName_ASCII)
{
    const bool use_long_format_names = false;
    const bool in_ASCII_8            = true;

    {
        auto data = cstr_array_view(
                "\x01\x00\x00\x00"
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
            );

        InStream in_stream(data);

        RDPECLIP::FormatListPDUEx format_list_pdu;

        format_list_pdu.recv(in_stream, use_long_format_names, in_ASCII_8);

        RED_CHECK_EQUAL(36, format_list_pdu.size(use_long_format_names));

        RED_CHECK(!format_list_pdu.will_be_sent_in_ASCII_8(use_long_format_names));

        auto format_name_it = format_list_pdu.begin();
        auto format_name_end = format_list_pdu.end();

        RED_CHECK_EQUAL(1, format_name_end - format_name_it);
        RED_CHECK_EQUAL(1, format_name_it->formatId());
        RED_CHECK_EQUAL("", format_name_it->format_name());
    }

    {
        auto data = cstr_array_view(
                "\x01\x00\x00\x00"
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                "\x02\x00\x00\x00"
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
            );

        InStream in_stream(data);

        RDPECLIP::FormatListPDUEx format_list_pdu;

        format_list_pdu.recv(in_stream, use_long_format_names, in_ASCII_8);

        RED_CHECK_EQUAL(72, format_list_pdu.size(use_long_format_names));

        RED_CHECK(!format_list_pdu.will_be_sent_in_ASCII_8(use_long_format_names));

        auto format_name_it = format_list_pdu.begin();
        auto format_name_end = format_list_pdu.end();

        RED_CHECK_EQUAL(2, format_name_end - format_name_it);

        RED_CHECK_EQUAL(1, format_name_it[0].formatId());
        RED_CHECK_EQUAL("", format_name_it[0].format_name());

        RED_CHECK_EQUAL(2, format_name_it[1].formatId());
        RED_CHECK_EQUAL("", format_name_it[1].format_name());
    }

    {
        auto data = cstr_array_view(
                "\x01\x00\x00\x00"
                "Test"
                                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
            );

        InStream in_stream(data);

        RDPECLIP::FormatListPDUEx format_list_pdu;

        format_list_pdu.recv(in_stream, use_long_format_names, in_ASCII_8);

        RED_CHECK(!format_list_pdu.will_be_sent_in_ASCII_8(use_long_format_names));

        RED_CHECK_EQUAL(36, format_list_pdu.size(use_long_format_names));

        auto format_name_it = format_list_pdu.begin();
        auto format_name_end = format_list_pdu.end();

        RED_CHECK_EQUAL(1, format_name_end - format_name_it);
        RED_CHECK_EQUAL(1, format_name_it->formatId());
        RED_CHECK_EQUAL("Test", format_name_it->format_name());
    }

    {
        auto data = cstr_array_view(
                "\x01\x00\x00\x00"
                "Test1"
                                    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"

                "\x02\x00\x00\x00"
                "Test2"
                                    "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
            );

        InStream in_stream(data);

        RDPECLIP::FormatListPDUEx format_list_pdu;

        format_list_pdu.recv(in_stream, use_long_format_names, in_ASCII_8);

        RED_CHECK_EQUAL(72, format_list_pdu.size(use_long_format_names));

        RED_CHECK(!format_list_pdu.will_be_sent_in_ASCII_8(use_long_format_names));

        auto format_name_it = format_list_pdu.begin();
        auto format_name_end = format_list_pdu.end();

        RED_CHECK_EQUAL(2, format_name_end - format_name_it);

        RED_CHECK_EQUAL(1, format_name_it[0].formatId());
        RED_CHECK_EQUAL("Test1", format_name_it[0].format_name());

        RED_CHECK_EQUAL(2, format_name_it[1].formatId());
        RED_CHECK_EQUAL("Test2", format_name_it[1].format_name());
    }
}

RED_AUTO_TEST_CASE(TestFormatListPDUEx_Recv_ShortFormatName_Unicode)
{
    const bool use_long_format_names = false;
    const bool in_ASCII_8            = false;

    {
        auto data = cstr_array_view(
                "\x01\x00\x00\x00"
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
            );

        InStream in_stream(data);

        RDPECLIP::FormatListPDUEx format_list_pdu;

        format_list_pdu.recv(in_stream, use_long_format_names, in_ASCII_8);

        RED_CHECK_EQUAL(36, format_list_pdu.size(use_long_format_names));

        RED_CHECK(!format_list_pdu.will_be_sent_in_ASCII_8(use_long_format_names));

        auto format_name_it = format_list_pdu.begin();
        auto format_name_end = format_list_pdu.end();

        RED_CHECK_EQUAL(1, format_name_end - format_name_it);
        RED_CHECK_EQUAL(1, format_name_it->formatId());
        RED_CHECK_EQUAL("", format_name_it->format_name());
    }

    {
        auto data = cstr_array_view(
                "\x01\x00\x00\x00"
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                "\x02\x00\x00\x00"
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
            );

        InStream in_stream(data);

        RDPECLIP::FormatListPDUEx format_list_pdu;

        format_list_pdu.recv(in_stream, use_long_format_names, in_ASCII_8);

        RED_CHECK_EQUAL(72, format_list_pdu.size(use_long_format_names));

        RED_CHECK(!format_list_pdu.will_be_sent_in_ASCII_8(use_long_format_names));

        auto format_name_it = format_list_pdu.begin();
        auto format_name_end = format_list_pdu.end();

        RED_CHECK_EQUAL(2, format_name_end - format_name_it);

        RED_CHECK_EQUAL(1, format_name_it[0].formatId());
        RED_CHECK_EQUAL("", format_name_it[0].format_name());

        RED_CHECK_EQUAL(2, format_name_it[1].formatId());
        RED_CHECK_EQUAL("", format_name_it[1].format_name());
    }

    {
        auto data = cstr_array_view(
                "\x01\x00\x00\x00"
                "T\x00" "e\x00" "s\x00" "t\x00"
                                                "\x00\x00\x00\x00\x00\x00\x00\x00"
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
            );

        InStream in_stream(data);

        RDPECLIP::FormatListPDUEx format_list_pdu;

        format_list_pdu.recv(in_stream, use_long_format_names, in_ASCII_8);

        RED_CHECK_EQUAL(36, format_list_pdu.size(use_long_format_names));

        RED_CHECK(!format_list_pdu.will_be_sent_in_ASCII_8(use_long_format_names));

        auto format_name_it = format_list_pdu.begin();
        auto format_name_end = format_list_pdu.end();

        RED_CHECK_EQUAL(1, format_name_end - format_name_it);
        RED_CHECK_EQUAL(1, format_name_it->formatId());
        RED_CHECK_EQUAL("Test", format_name_it->format_name());
    }

    {
        auto data = cstr_array_view(
                "\x01\x00\x00\x00"
                "T\x00" "e\x00" "s\x00" "t\x00" "1\x00"
                                                        "\x00\x00\x00\x00\x00\x00"
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"

                "\x02\x00\x00\x00"
                "T\x00" "e\x00" "s\x00" "t\x00" "2\x00"
                                                        "\x00\x00\x00\x00\x00\x00"
                "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
            );

        InStream in_stream(data);

        RDPECLIP::FormatListPDUEx format_list_pdu;

        format_list_pdu.recv(in_stream, use_long_format_names, in_ASCII_8);

        RED_CHECK_EQUAL(72, format_list_pdu.size(use_long_format_names));

        RED_CHECK(!format_list_pdu.will_be_sent_in_ASCII_8(use_long_format_names));

        auto format_name_it = format_list_pdu.begin();
        auto format_name_end = format_list_pdu.end();

        RED_CHECK_EQUAL(2, format_name_end - format_name_it);

        RED_CHECK_EQUAL(1, format_name_it[0].formatId());
        RED_CHECK_EQUAL("Test1", format_name_it[0].format_name());

        RED_CHECK_EQUAL(2, format_name_it[1].formatId());
        RED_CHECK_EQUAL("Test2", format_name_it[1].format_name());
    }
}
