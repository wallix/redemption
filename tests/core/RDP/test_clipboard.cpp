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
#include "system/redemption_unit_tests.hpp"

#include "utils/hexdump.hpp"
#include "core/RDP/clipboard.hpp"


RED_AUTO_TEST_CASE(TestFormatDataResponsePDU)
{
    {
        //  emit_fileList
        StaticOutStream<1024> ou_stream_fileList;

        uint8_t UTF16nameData[20];
        std::string nameUTF8("abcde.txt");
        int UTF16nameSize = ::UTF8toUTF16(reinterpret_cast<const uint8_t *>(nameUTF8.c_str()), UTF16nameData, nameUTF8.size() *2);
        std::string nameUTF16 = std::string(reinterpret_cast<char *>(UTF16nameData), UTF16nameSize);
        std::string name = nameUTF16 ;
        //uint64_t size = 17 ;
        const int cItems = 1;

        RDPECLIP::FormatDataResponsePDU_FileList fdr(cItems);
        fdr.emit(ou_stream_fileList);

        const uint8_t file_list_data[] =
            "\x05\x00\x01\x00\x54\x02\x00\x00\x01\x00\x00\x00";

        std::string const out_data(reinterpret_cast<char *>(ou_stream_fileList.get_data()), 12);
        std::string const expected(reinterpret_cast<const char *>(file_list_data), 12);
        RED_CHECK_EQUAL(expected, out_data);
    }


    {
        //  recv_fileList
        uint8_t UTF16nameData[20];
        std::string nameUTF8("abcde.txt");
        int UTF16nameSize = ::UTF8toUTF16(reinterpret_cast<const uint8_t *>(nameUTF8.c_str()), UTF16nameData, nameUTF8.size() *2);
        std::string nameUTF16 = std::string(reinterpret_cast<char *>(UTF16nameData), UTF16nameSize);

        std::string name = nameUTF16 ;
        const int cItems = 1;

        const uint8_t file_list_data[] =
            "\x05\x00\x01\x00\x54\x02\x00\x00\x01\x00\x00\x00\x64\x40\x00\x00\x00\x00\x00\x00"
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
            "\x00\x00\x00\x00\x00\x00\x00\x00";

        InStream in_stream_fileList_to_recv(file_list_data, 608);
        RDPECLIP::FormatDataResponsePDU_FileList fdr_recv;
        fdr_recv.recv(in_stream_fileList_to_recv);

        RED_CHECK_EQUAL(fdr_recv.cItems, cItems);
    }


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

        std::string out_data(reinterpret_cast<char *>(ou_stream_metaFilePic.get_data()), 132);

        const char metafilepic_out_data[] =
            "\x05\x00\x01\x00\xb6\xbc\x00\x00\x08\x00\x00\x00\x60\x22\x00\x00"
            "\x68\x0b\x00\x00\x01\x00\x09\x00\x00\x03\x55\x5e\x00\x00\x00\x00"
            "\x3b\x5e\x00\x00\x00\x00\x04\x00\x00\x00\x03\x01\x08\x00\x05\x00"
            "\x00\x00\x0c\x02\xb7\xff\xdc\x00\x05\x00\x00\x00\x0b\x02\x00\x00"
            "\x00\x00\x3b\x5e\x00\x00\x41\x0b\x20\x00\xcc\x00\x49\x00\xdc\x00"
            "\x00\x00\x00\x00\xb7\xff\xdc\x00\x00\x00\x00\x00\x28\x00\x00\x00"
            "\xdc\x00\x00\x00\xb7\xff\xff\xff\x01\x00\x18\x00\x00\x00\x00\x00"
            "\x34\xbc\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
            "\x00\x00\x00\x00";

        std::string expected(reinterpret_cast<const char *>(metafilepic_out_data), 132);

        RED_CHECK_EQUAL(expected, out_data);

    }


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

        InStream stream(metafilepic_out_data, 132);

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
//     hexdump(out_stream.get_data(), out_stream.get_offset());

    RED_CHECK_EQUAL(out_stream.get_offset(), in_stream.get_offset());
    RED_CHECK_EQUAL(0, memcmp(in_data, out_data, sizeof(in_data) - 1));
}



RED_AUTO_TEST_CASE(TestFormatListPDU) {

    // inData
    std::string name1("FileContents");
    std::string name2("FileGroupDescriptorW");
    std::string name3("\0\0", 2);

    // Init stream format list PDU long name
    StaticOutStream<1024> out_stream;
    RDPECLIP::CliprdrHeader header(RDPECLIP::CB_FORMAT_LIST, 0, 0);

    RDPECLIP::FormatListPDU_LongName format_list_pdu_long0(48026,
                                                           name1.c_str(),
                                                           name1.size());
    header.dataLen_ += format_list_pdu_long0.formatDataNameUTF16Len + 4;
    RDPECLIP::FormatListPDU_LongName format_list_pdu_long1(48025,
                                                           name2.c_str(),
                                                           name2.size());
    header.dataLen_ += format_list_pdu_long1.formatDataNameUTF16Len + 4;
    RDPECLIP::FormatListPDU_LongName format_list_pdu_long2(RDPECLIP::CF_UNICODETEXT,
                                                           name3.c_str(),
                                                           name3.size());
    header.dataLen_ += format_list_pdu_long2.formatDataNameUTF16Len + 4;
    RDPECLIP::FormatListPDU_LongName format_list_pdu_long3(RDPECLIP::CF_TEXT,
                                                           name3.c_str(),
                                                           name3.size());
    header.dataLen_ += format_list_pdu_long3.formatDataNameUTF16Len + 4;
    RDPECLIP::FormatListPDU_LongName format_list_pdu_long4(RDPECLIP::CF_METAFILEPICT,
                                                           name3.c_str(),
                                                           name3.size());
    header.dataLen_ += format_list_pdu_long4.formatDataNameUTF16Len + 4;

    header.emit(out_stream);
    format_list_pdu_long0.emit(out_stream);
    format_list_pdu_long1.emit(out_stream);
    format_list_pdu_long2.emit(out_stream);
    format_list_pdu_long3.emit(out_stream);
    format_list_pdu_long4.emit(out_stream);

//     header.log();
//     format_list_pdu_long0.log();
//     format_list_pdu_long1.log();
//     format_list_pdu_long2.log();
//     format_list_pdu_long3.log();
//     format_list_pdu_long4.log();
//
//     hexdump(out_stream.get_data(), out_stream.get_offset(), 16);


    const char exp_data[] =
        "\x02\x00\x00\x00\x5e\x00\x00\x00\x9a\xbb\x00\x00\x46\x00\x69\x00" //....^.......F.i.
        "\x6c\x00\x65\x00\x43\x00\x6f\x00\x6e\x00\x74\x00\x65\x00\x6e\x00" //l.e.C.o.n.t.e.n.
        "\x74\x00\x73\x00\x00\x00\x99\xbb\x00\x00\x46\x00\x69\x00\x6c\x00" //t.s.......F.i.l.
        "\x65\x00\x47\x00\x72\x00\x6f\x00\x75\x00\x70\x00\x44\x00\x65\x00" //e.G.r.o.u.p.D.e.
        "\x73\x00\x63\x00\x72\x00\x69\x00\x70\x00\x74\x00\x6f\x00\x72\x00" //s.c.r.i.p.t.o.r.
        "\x57\x00\x00\x00\x0d\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00" //W...............
        "\x03\x00\x00\x00\x00\x00";



    std::string expected(reinterpret_cast<const char *>(exp_data), sizeof(exp_data)-1);
    std::string out_data(reinterpret_cast<char *>(out_stream.get_data()), out_stream.get_offset());

    RED_CHECK_EQUAL(expected, out_data);
}



RED_AUTO_TEST_CASE(TestFileContentsRequestPDU) {

    { // FILECONTENTS_RANGE emit TEST
    // inData
    uint32_t ID = 1;
    uint32_t index = 3;
    uint32_t flag = RDPECLIP::FILECONTENTS_RANGE;
    uint64_t size = 0x0000000000000007;

    // Init Stream
    StaticOutStream<64> out_stream;
    RDPECLIP::FileContentsRequestPDU fileContentsRequest(ID, flag, index, size, size);
    fileContentsRequest.emit(out_stream);

    const char exp_data[] =
        "\x08\x00\x00\x00\x1c\x00\x00\x00\x01\x00\x00\x00\x03\x00\x00\x00"
        "\x02\x00\x00\x00\x07\x00\x00\x00\x00\x00\x00\x00\x07\x00\x00\x00"
        "\x00\x00\x00\x00";

    std::string expected(reinterpret_cast<const char *>(exp_data), sizeof(exp_data)-1);
    std::string out_data(reinterpret_cast<char *>(out_stream.get_data()), out_stream.get_offset());
//    hexdump(out_stream.get_data(), out_stream.get_offset());

    RED_CHECK_EQUAL(expected, out_data);
    }


    { // FILECONTENTS_RANGE recv TEST
    // inData
    uint32_t ID = 1;
    uint32_t index = 3;
    uint32_t flag = RDPECLIP::FILECONTENTS_RANGE;
    uint64_t size = 0x0000000000000007;

    const char data[] =
        "\x08\x00\x00\x00\x1c\x00\x00\x00\x01\x00\x00\x00\x03\x00\x00\x00"
        "\x02\x00\x00\x00\x07\x00\x00\x00\x00\x00\x00\x00\x07\x00\x00\x00"
        "\x01\x00\x00\x00";

    InStream stream(data, sizeof(data)-1);
    RDPECLIP::FileContentsRequestPDU fileContentsRequest;
    fileContentsRequest.recv(stream);

    RED_CHECK_EQUAL(fileContentsRequest.streamID, ID);
    RED_CHECK_EQUAL(fileContentsRequest.flag, flag);
    RED_CHECK_EQUAL(fileContentsRequest.lindex, index);
    RED_CHECK_EQUAL(fileContentsRequest.sizeRequested, size);

    }


    { // FILECONTENTS_SIZE emit TEST
        // inData
    uint32_t ID = 1;
    uint32_t index = 3;
    uint32_t flag = RDPECLIP::FILECONTENTS_SIZE;
    uint64_t size = 0x0000000000000007;

    // Init Stream
    StaticOutStream<64> out_stream;
    RDPECLIP::FileContentsRequestPDU fileContentsRequest(ID, flag, index, size, size);
    fileContentsRequest.emit(out_stream);

    const char exp_data[] =
        "\x08\x00\x00\x00\x1c\x00\x00\x00\x01\x00\x00\x00\x03\x00\x00\x00"
        "\x01\x00\x00\x00\x07\x00\x00\x00\x00\x00\x00\x00\x08\x00\x00\x00"
        "\x00\x00\x00\x00";

    std::string expected(reinterpret_cast<const char *>(exp_data), sizeof(exp_data)-1);
    std::string out_data(reinterpret_cast<char *>(out_stream.get_data()), out_stream.get_offset());

    RED_CHECK_EQUAL(expected, out_data);
    }


    { // FILECONTENTS_SIZE recv TEST
    // inData
    uint32_t ID = 1;
    uint32_t index = 3;
    uint32_t flag = RDPECLIP::FILECONTENTS_SIZE;
    uint64_t size = 0x0000000000000007;

    const char data[] =
        "\x08\x00\x00\x00\x1c\x00\x00\x00\x01\x00\x00\x00\x03\x00\x00\x00"
        "\x01\x00\x00\x00\x07\x00\x00\x00\x00\x00\x00\x00\x08\x00\x00\x00"
        "\x01\x00\x00\x00";

    InStream stream(data, sizeof(data)-1);
    RDPECLIP::FileContentsRequestPDU fileContentsRequest;
    fileContentsRequest.recv(stream);

    RED_CHECK_EQUAL(fileContentsRequest.streamID, ID);
    RED_CHECK_EQUAL(fileContentsRequest.flag, flag);
    RED_CHECK_EQUAL(fileContentsRequest.lindex, index);
    RED_CHECK_EQUAL(fileContentsRequest.sizeRequested, size);

    }
}




