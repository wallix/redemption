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

//#define LOGNULL
#define LOGPRINT

#include "core/WMF/MetaFileFormat.hpp"

#include <iostream>


BOOST_AUTO_TEST_CASE(TestFormatDataResponsePDU)
{
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

        InStream stream(metafilepic_out_data, 132);
/*
        RDPECLIP::FormatDataResponsePDU_MetaFilePic fdr;
        fdr.recv(stream);

        BOOST_CHECK_EQUAL(fdr.mappingMode, RDPECLIP::MM_ANISOTROPIC);
        BOOST_CHECK_EQUAL(fdr.xExt, width * ARBITRARY_SCALE);
        BOOST_CHECK_EQUAL(fdr.yExt, height * ARBITRARY_SCALE);

        RDPECLIP::FormatDataResponsePDU_MetaFilePic::MetaHeader & metaHeader = fdr.metaHeader;
        BOOST_CHECK_EQUAL(metaHeader.type, RDPECLIP::MEMORYMETAFILE);
        BOOST_CHECK_EQUAL(metaHeader.headerSize, unsigned(RDPECLIP::FormatDataResponsePDU_MetaFilePic::MetaHeader::HEADER_SIZE));
        BOOST_CHECK_EQUAL(metaHeader.version, RDPECLIP::METAVERSION300);
        BOOST_CHECK_EQUAL(metaHeader.size, (data_lenght/2) + RDPECLIP::METAFILE_WORDS_HEADER_SIZE);
        BOOST_CHECK_EQUAL(metaHeader.numberOfObjects, 0);
        BOOST_CHECK_EQUAL(metaHeader.maxRecord, (data_lenght + RDPECLIP::META_DIBSTRETCHBLT_HEADER_SIZE)/2);
        BOOST_CHECK_EQUAL(metaHeader.numberOfMembers, 0);

        RDPECLIP::FormatDataResponsePDU_MetaFilePic::MetaSetWindowExt & metaSetWindowExt = fdr.metaSetWindowExt;
        BOOST_CHECK_EQUAL(metaSetWindowExt.height, negative_height16);
        BOOST_CHECK_EQUAL(metaSetWindowExt.width, width);

        RDPECLIP::FormatDataResponsePDU_MetaFilePic::MetaSetWindowOrg & metaSetWindowOrg = fdr.metaSetWindowOrg;
        BOOST_CHECK_EQUAL(metaSetWindowOrg.yOrg, 0);
        BOOST_CHECK_EQUAL(metaSetWindowOrg.xOrg, 0);

        RDPECLIP::FormatDataResponsePDU_MetaFilePic::MetaSetMepMod & metaSetMepMod = fdr.metaSetMepMod;
        BOOST_CHECK_EQUAL(metaSetMepMod.mappingMode, RDPECLIP::MM_ANISOTROPIC);

        RDPECLIP::FormatDataResponsePDU_MetaFilePic::DibStretchBLT & dibStretchBLT = fdr.dibStretchBLT;
        BOOST_CHECK_EQUAL(dibStretchBLT.recordSize, (data_lenght + RDPECLIP::META_DIBSTRETCHBLT_HEADER_SIZE)/2);
        BOOST_CHECK_EQUAL(dibStretchBLT.recordFunction, MFF::META_DIBSTRETCHBLT);
        BOOST_CHECK_EQUAL(dibStretchBLT.rasterOperation, RDPECLIP::SRCCOPY);
        BOOST_CHECK_EQUAL(dibStretchBLT.srcHeight, height);
        BOOST_CHECK_EQUAL(dibStretchBLT.srcWidth, width);
        BOOST_CHECK_EQUAL(dibStretchBLT.ySrc, 0);
        BOOST_CHECK_EQUAL(dibStretchBLT.xSrc, 0);
        BOOST_CHECK_EQUAL(dibStretchBLT.destHeight, negative_height16);
        BOOST_CHECK_EQUAL(dibStretchBLT.destWidth, width);
        BOOST_CHECK_EQUAL(dibStretchBLT.yDest, 0);
        BOOST_CHECK_EQUAL(dibStretchBLT.xDest, 0);

        RDPECLIP::FormatDataResponsePDU_MetaFilePic::DibStretchBLT::BitmapInfoHeader & bitmapInfoHeader = dibStretchBLT.bitmapInfoHeader;
        BOOST_CHECK_EQUAL(bitmapInfoHeader.headerSize, 40);
        BOOST_CHECK_EQUAL(bitmapInfoHeader.height, negative_height32);
        BOOST_CHECK_EQUAL(bitmapInfoHeader.width, width);
        BOOST_CHECK_EQUAL(bitmapInfoHeader.planes, 0x0001);
        BOOST_CHECK_EQUAL(bitmapInfoHeader.bitCount, bpp);
        BOOST_CHECK_EQUAL(bitmapInfoHeader.compression, 0);
        BOOST_CHECK_EQUAL(bitmapInfoHeader.imageSize, data_lenght);
        BOOST_CHECK_EQUAL(bitmapInfoHeader.xPelsPerMeter, 0);
        BOOST_CHECK_EQUAL(bitmapInfoHeader.yPelsPerMeter, 0);
        BOOST_CHECK_EQUAL(bitmapInfoHeader.colorUsed, 0);
        BOOST_CHECK_EQUAL(bitmapInfoHeader.colorImportant, 0);*/
    }
}


