/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2010
   Author(s): Christophe Grosjean, Javier Caverni
   Based on xrdp Copyright (C) Jay Sorg 2004-2010

   Unit test to writing RDP orders to file and rereading them

*/

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TestRdesktopInvalidPDU
#include <boost/test/auto_unit_test.hpp>
#include "./test_orders.hpp"

#include "stream.hpp"
#include "transport.hpp"
#include "constants.hpp"
#include "RDP/x224.hpp"
#include "RDP/mcs.hpp"
#include "RDP/sec.hpp"


BOOST_AUTO_TEST_CASE(TestDecodePacket)
{
    Stream stream(65536);
    GeneratorTransport t(
        "\x03\x00\x00\x3d"
        "\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x80\x2e\x08\x00\x00\x00\x05"
        "\xc2\x9f\x8f\x9c\x6a\xc1\xf0\x2e\x14\xba\x9f\x05\xd2\x72\xc7\xe9"
        "\x63\x15\x71\xd5\x73\xb0\xfe\xe7\xba\x9e\x75\xa0\x68\x67\xa9\x3f"
        "\x7b\xfa\x9a\x3b\x15\x9f\x89\x32\xd4"
        , 61);

    X224In tpdu(&t, stream);
    BOOST_CHECK_EQUAL(3, tpdu.tpkt.version);
    BOOST_CHECK_EQUAL(61, tpdu.tpkt.len);
    BOOST_CHECK_EQUAL(2, tpdu.tpdu_hdr.LI);
    BOOST_CHECK_EQUAL((uint8_t)X224Packet::DT_TPDU, (uint8_t)tpdu.tpdu_hdr.code);

    McsIn mcs_in(stream);
    BOOST_CHECK_EQUAL((uint8_t)MCS_SDRQ, (uint8_t)mcs_in.opcode >> 2);
    BOOST_CHECK_EQUAL(0, mcs_in.user_id);
    BOOST_CHECK_EQUAL((uint16_t)MCS_GLOBAL_CHANNEL, (uint16_t)mcs_in.chan_id);
    BOOST_CHECK_EQUAL(0x70, mcs_in.magic_0x70);
    BOOST_CHECK_EQUAL(46, mcs_in.len);

    CryptContext decrypt;

    BOOST_CHECK_EQUAL(1032, sizeof(decrypt.rc4_info));

    decrypt.use_count=1;
    memcpy(decrypt.sign_key, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 16);
    memcpy(decrypt.key,      "\xd1\x26\x9e\x80\x59\xa3\x09\x2e\x10\xf2\x8a\xf0\xf3\x41\xbe\xfc", 16);
    decrypt.rc4_key_len=8;

    // dirty trick to guess internal openssl memory layout for rc4_info structure...
    // yes it's black magic
    SSL_RC4 rc4_info;
    RC4_set_key(&rc4_info, 8, (const unsigned char*)"\x10ZZZZZZZZ");

    if (rc4_info.data[0] == 0x10){
        memcpy((void*)(&decrypt.rc4_info), (void*)
        "\x32\x00\x00\x00\x30\x00\x00\x00"

        "\x71\0\0\0\xf5\0\0\0\xf1\0\0\0\x61\0\0\0\x62\0\0\0\x7f\0\0\0\xa8\0\0\0\xc4\0\0\0"
        "\x47\0\0\0\x27\0\0\0\x48\0\0\0\xac\0\0\0\x30\0\0\0\x91\0\0\0\x5a\0\0\0\xaa\0\0\0"
        "\x92\0\0\0\x55\0\0\0\xe0\0\0\0\x66\0\0\0\x2b\0\0\0\x83\0\0\0\xdc\0\0\0\x0f\0\0\0"
        "\x53\0\0\0\xd4\0\0\0\xe6\0\0\0\xb7\0\0\0\xb5\0\0\0\xb9\0\0\0\x80\0\0\0\xe7\0\0\0"

        "\xfa\0\0\0\x98\0\0\0\x6a\0\0\0\xe2\0\0\0\x73\0\0\0\xe5\0\0\0\x1d\0\0\0\x04\0\0\0"
        "\xea\0\0\0\x05\0\0\0\xf4\0\0\0\xf6\0\0\0\x74\0\0\0\x6e\0\0\0\xc0\0\0\0\x69\0\0\0"
        "\x29\0\0\0\x10\0\0\0\x79\0\0\0\xb0\0\0\0\x7d\0\0\0\x16\0\0\0\x52\0\0\0\x95\0\0\0"
        "\x59\0\0\0\x00\0\0\0\xcc\0\0\0\x84\0\0\0\xc7\0\0\0\x07\0\0\0\xc3\0\0\0\xda\0\0\0"

        "\xcb\0\0\0\x0e\0\0\0\xba\0\0\0\xc8\0\0\0\xe3\0\0\0\x88\0\0\0\xec\0\0\0\xfd\0\0\0"
        "\xd6\0\0\0\x7e\0\0\0\xce\0\0\0\xd2\0\0\0\xfb\0\0\0\x19\0\0\0\xad\0\0\0\x4c\0\0\0"
        "\xbd\0\0\0\x3c\0\0\0\x8e\0\0\0\x8d\0\0\0\x64\0\0\0\x81\0\0\0\x28\0\0\0\x46\0\0\0"
        "\x5c\0\0\0\x72\0\0\0\xb1\0\0\0\xff\0\0\0\x58\0\0\0\x60\0\0\0\x5e\0\0\0\xeb\0\0\0"

        "\xe4\0\0\0\x1a\0\0\0\x50\0\0\0\xee\0\0\0\xbf\0\0\0\x90\0\0\0\xae\0\0\0\x86\0\0\0"
        "\x94\0\0\0\x65\0\0\0\x54\0\0\0\xf7\0\0\0\x31\0\0\0\x0a\0\0\0\x15\0\0\0\xd9\0\0\0"
        "\x33\0\0\0\xbe\0\0\0\x06\0\0\0\x38\0\0\0\xf2\0\0\0\x49\0\0\0\xb8\0\0\0\x03\0\0\0"
        "\x6d\0\0\0\x02\0\0\0\xde\0\0\0\xfc\0\0\0\x5d\0\0\0\x7c\0\0\0\x2a\0\0\0\x70\0\0\0"

        "\xa2\0\0\0\xcd\0\0\0\x36\0\0\0\x21\0\0\0\x13\0\0\0\x09\0\0\0\x41\0\0\0\x93\0\0\0"
        "\x7a\0\0\0\x51\0\0\0\x78\0\0\0\xaf\0\0\0\x2e\0\0\0\xdf\0\0\0\xb6\0\0\0\xd8\0\0\0"
        "\x9c\0\0\0\xe1\0\0\0\x85\0\0\0\x9b\0\0\0\x77\0\0\0\x3e\0\0\0\xa6\0\0\0\x8a\0\0\0"
        "\x12\0\0\0\xf3\0\0\0\x63\0\0\0\xcf\0\0\0\xab\0\0\0\x6f\0\0\0\xf0\0\0\0\x89\0\0\0"

        "\x9d\0\0\0\x1c\0\0\0\x4e\0\0\0\x76\0\0\0\x0b\0\0\0\xd0\0\0\0\xd7\0\0\0\xd5\0\0\0"
        "\x87\0\0\0\x43\0\0\0\x08\0\0\0\x23\0\0\0\x1e\0\0\0\x17\0\0\0\x2d\0\0\0\xfe\0\0\0"
        "\x3f\0\0\0\xf9\0\0\0\xc5\0\0\0\x68\0\0\0\x35\0\0\0\x3b\0\0\0\xb2\0\0\0\xa9\0\0\0"
        "\x8f\0\0\0\x99\0\0\0\x40\0\0\0\xc9\0\0\0\x5f\0\0\0\x45\0\0\0\x97\0\0\0\xbb\0\0\0"

        "\xa7\0\0\0\xd1\0\0\0\x2f\0\0\0\xc2\0\0\0\xdd\0\0\0\x96\0\0\0\x9e\0\0\0\x11\0\0\0"
        "\x24\0\0\0\x57\0\0\0\xd3\0\0\0\xa5\0\0\0\xa3\0\0\0\xe8\0\0\0\xa0\0\0\0\x82\0\0\0"
        "\x3a\0\0\0\x67\0\0\0\x9a\0\0\0\xed\0\0\0\x01\0\0\0\x8b\0\0\0\x18\0\0\0\xa4\0\0\0"
        "\x2c\0\0\0\x4a\0\0\0\x39\0\0\0\x5b\0\0\0\xf8\0\0\0\x14\0\0\0\x44\0\0\0\x20\0\0\0"

        "\x34\0\0\0\x8c\0\0\0\x0d\0\0\0\x25\0\0\0\x0c\0\0\0\xe9\0\0\0\x75\0\0\0\x6b\0\0\0"
        "\x9f\0\0\0\xb4\0\0\0\x1f\0\0\0\xef\0\0\0\xbc\0\0\0\xca\0\0\0\x3d\0\0\0\xc1\0\0\0"
        "\x22\0\0\0\x32\0\0\0\xb3\0\0\0\xa1\0\0\0\xc6\0\0\0\x4d\0\0\0\x6c\0\0\0\x7b\0\0\0"
        "\xdb\0\0\0\x4f\0\0\0\x37\0\0\0\x1b\0\0\0\x56\0\0\0\x4b\0\0\0\x26\0\0\0\x42\0\0\0"

        "\xff\0\0\0\xff\0\0\0\xff\0\0\0\xff\0\0\0\x01\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0"
        "\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0"
        "\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0"
        "\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0"

        "\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0"
        "\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0"
        "\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0"
        "\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0"
        , 1032);
    }
    else {
        memcpy((void*)(&decrypt.rc4_info), (void*)
            "\x32\x00\x00\x00\x30\x00\x00\x00\x71\xf5\xf1\x61\x62\x7f\xa8\xc4\x47\x27\x48\xac\x30\x91\x5a\xaa\x92\x55\xe0\x66\x2b\x83\xdc\x0f\x53\xd4\xe6\xb7\xb5\xb9\x80\xe7\xfa\x98\x6a\xe2\x73\xe5\x1d\x04\xea\x05\xf4\xf6\x74\x6e\xc0\x69\x29\x10\x79\xb0\x7d\x16\x52\x95\x59\x00\xcc\x84\xc7\x07\xc3\xda\xcb\x0e\xba\xc8\xe3\x88\xec\xfd\xd6\x7e\xce\xd2\xfb\x19\xad\x4c\xbd\x3c\x8e\x8d\x64\x81\x28\x46\x5c\x72\xb1\xff\x58\x60\x5e\xeb\xe4\x1a\x50\xee\xbf\x90\xae\x86\x94\x65\x54\xf7\x31\x0a\x15\xd9\x33\xbe\x06\x38\xf2\x49\xb8\x03\x6d\x02\xde\xfc\x5d\x7c\x2a\x70\xa2\xcd\x36\x21\x13\x09\x41\x93\x7a\x51\x78\xaf\x2e\xdf\xb6\xd8\x9c\xe1\x85\x9b\x77\x3e\xa6\x8a\x12\xf3\x63\xcf\xab\x6f\xf0\x89\x9d\x1c\x4e\x76\x0b\xd0\xd7\xd5\x87\x43\x08\x23\x1e\x17\x2d\xfe\x3f\xf9\xc5\x68\x35\x3b\xb2\xa9\x8f\x99\x40\xc9\x5f\x45\x97\xbb\xa7\xd1\x2f\xc2\xdd\x96\x9e\x11\x24\x57\xd3\xa5\xa3\xe8\xa0\x82\x3a\x67\x9a\xed\x01\x8b\x18\xa4\x2c\x4a\x39\x5b\xf8\x14\x44\x20\x34\x8c\x0d\x25\x0c\xe9\x75\x6b\x9f\xb4\x1f\xef\xbc\xca\x3d\xc1\x22\x32\xb3\xa1\xc6\x4d\x6c\x7b\xdb\x4f\x37\x1b\x56\x4b\x26\x42\xff\xff\xff\xff\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 1032);
    }
    decrypt.rc4_key_size=1;

    SecIn sec(stream, decrypt);
    BOOST_CHECK_EQUAL((uint32_t)SEC_ENCRYPT, (uint32_t)sec.flags);

    uint16_t length = stream.in_uint16_le();
    BOOST_CHECK_EQUAL((uint16_t)34, length);

}

// trace of process logon info

//rdpproxy: WARNING (20016/20016) -- Reading performance flags
//rdpproxy: core/stream.hpp:158: unsigned int Stream::in_uint32_le(): Assertion `check_rem(4)' failed.

BOOST_AUTO_TEST_CASE(TestDecodeProcessLogonInfoPacket)
{
    Stream stream(65536);
    GeneratorTransport t(
/* 0000 */"\x03\x00\x01\x4d"                                                 //...M
// Socket RDP Client (5) receiving 329 bytes
/* 0000 */"\x02\xf0\x80\x64\x00\x00\x03\xeb\x70\x81\x3e\x48\x00\x00\x00\x96" //...d....p.>H....
/* 0010 */"\x4e\xca\x54\x28\x3c\x2d\xde\x83\x0a\xac\xbe\x8d\x20\x70\x25\x95" //N.T(<-...... p%.
/* 0020 */"\xbb\x3a\x69\x4b\xfb\x52\x7f\x11\x3b\x9d\x40\xf9\x70\x8b\xfc\x0b" //.:iK.R..;.@.p...
/* 0030 */"\xe1\xe4\x95\x3e\x48\x38\x89\xca\x37\x7b\xb8\x2a\x2a\x86\xb4\xca" //...>H8..7{.**...
/* 0040 */"\xd3\xb3\x3b\x0e\x87\xd2\x9c\xd9\x1c\x5e\xb8\xa1\x0c\x53\xa3\x22" //..;......^...S."
/* 0050 */"\x35\x67\x7e\x87\x88\x44\xa2\x32\x1c\x3d\xcc\x77\x47\xf2\x3a\xd6" //5g~..D.2.=.wG.:.
/* 0060 */"\xfe\x8d\x7c\xb7\x12\xc4\x6a\x64\x22\x1b\x6d\x23\x8c\x05\x97\xd6" //..|...jd".m#....
/* 0070 */"\x43\x9e\x46\x84\x41\xd1\x20\xf0\xfd\x5d\x11\x01\x67\xb9\xcb\x1a" //C.F.A. ..]..g...
/* 0080 */"\xde\xb1\x49\x0a\xd2\x34\x76\x84\xc6\x62\xda\xdb\x82\x20\x68\xf5" //..I..4v..b... h.
/* 0090 */"\x89\x89\x28\xbe\x9f\x61\xdb\x0b\x8b\x43\x10\xb5\xac\x15\x7a\x79" //..(..a...C....zy
/* 00a0 */"\xcb\x00\x2c\xa2\x19\x19\xc4\x44\x9c\xa4\xea\xdf\x69\x7a\xb4\xbe" //..,....D....iz..
/* 00b0 */"\x74\xeb\x39\xb0\xd5\x03\x2d\x81\x96\x25\x05\x7d\xc8\x8c\xa1\xe1" //t.9...-..%.}....
/* 00c0 */"\x24\xdd\x76\x47\x00\x17\xf3\x0c\x5c\xd3\x22\xa1\x41\x6e\x54\x3c" //$.vG....\.".AnT<
/* 00d0 */"\x28\xa2\x9f\x37\x2f\xb5\x64\x17\x54\x26\x97\xf5\x39\x83\xed\x3f" //(..7/.d.T&..9..?
/* 00e0 */"\x1f\x3f\xf5\x8e\xab\xa7\x82\x8c\x77\x81\x7d\x80\xd0\x73\x0e\xb7" //.?......w.}..s..
/* 00f0 */"\x6b\xc3\xd0\xc6\x51\x61\x29\x6a\x3d\x9d\x99\x22\x3e\x78\x7a\x77" //k...Qa)j=..">xzw
/* 0100 */"\x63\x07\x01\xca\x14\x26\xcd\x6a\xc0\x0d\xa7\x91\x1c\x7f\x6d\xf4" //c....&.j......m.
/* 0110 */"\xfa\x3d\x26\xe8\xf8\x4c\x5f\xe9\x78\xe6\xde\x09\x79\xb3\x50\x38" //.=&..L_.x...y.P8
/* 0120 */"\x72\x52\x1f\x7f\x33\x2b\xc6\xbd\x55\x62\xf9\xe8\xae\xb1\xd8\xad" //rR..3+..Ub......
/* 0130 */"\x6c\xac\x6b\xc4\xe7\x90\x73\x9b\xea\xc1\x53\xe2\x37\xd4\x67\x81" //l.k...s...S.7.g.
/* 0140 */"\x39\x1b\xac\xe8\xf9\x4d\x67\x70\x44"                             //9....MgpD
        , 333);

    X224In tpdu(&t, stream);
    BOOST_CHECK_EQUAL(3, tpdu.tpkt.version);
    BOOST_CHECK_EQUAL(333, tpdu.tpkt.len);
    BOOST_CHECK_EQUAL(2, tpdu.tpdu_hdr.LI);
    BOOST_CHECK_EQUAL((uint8_t)X224Packet::DT_TPDU, (uint8_t)tpdu.tpdu_hdr.code);

    McsIn mcs_in(stream);
    BOOST_CHECK_EQUAL((uint8_t)MCS_SDRQ, mcs_in.opcode >> 2);
    BOOST_CHECK_EQUAL(0, mcs_in.user_id);
    BOOST_CHECK_EQUAL((uint16_t)MCS_GLOBAL_CHANNEL, (uint16_t)mcs_in.chan_id);
    BOOST_CHECK_EQUAL(0x70, mcs_in.magic_0x70);
    BOOST_CHECK_EQUAL(318, mcs_in.len);

    CryptContext decrypt;
    decrypt.use_count=0;
    memcpy(decrypt.sign_key, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 16);
    memcpy(decrypt.key, "\xd1\x26\x9e\x77\x53\x54\x03\xd2\xaf\x3c\x95\xba\xd7\x48\xb3\xaa", 16);
    decrypt.rc4_key_len=8;
    // dirty trick to guess internal openssl memory layout for rc4_info structure...
    // yes it's black magic
    SSL_RC4 rc4_info;
    RC4_set_key(&rc4_info, 8, (const unsigned char*)"\x10ZZZZZZZZ");

    if (rc4_info.data[0] == 0x10){
    memcpy((void*)(&decrypt.rc4_info), (void*)"\x00\x00\x00\x00\x00\x00\x00\x00"
        "\x64\0\0\0\x1f\0\0\0\x98\0\0\0\x12\0\0\0\xbc\0\0\0\x6d\0\0\0\xcb\0\0\0\xa4\0\0\0"
        "\x27\0\0\0\x47\0\0\0\x24\0\0\0\xe0\0\0\0\xab\0\0\0\x96\0\0\0\xf7\0\0\0\x0d\0\0\0"
        "\x4e\0\0\0\xa0\0\0\0\xb2\0\0\0\xdc\0\0\0\x32\0\0\0\x9b\0\0\0\x94\0\0\0\x90\0\0\0"
        "\x8d\0\0\0\xc5\0\0\0\x5a\0\0\0\x95\0\0\0\x2c\0\0\0\xef\0\0\0\x04\0\0\0\x3e\0\0\0"

        "\x9a\0\0\0\x39\0\0\0\x9f\0\0\0\xde\0\0\0\x0c\0\0\0\xc7\0\0\0\x09\0\0\0\xa5\0\0\0"
        "\x22\0\0\0\xed\0\0\0\xb5\0\0\0\x57\0\0\0\x67\0\0\0\x2b\0\0\0\x0f\0\0\0\x8f\0\0\0"
        "\x8a\0\0\0\xe1\0\0\0\x23\0\0\0\x2e\0\0\0\x87\0\0\0\x54\0\0\0\x5d\0\0\0\x8c\0\0\0"
        "\x6f\0\0\0\x81\0\0\0\x8e\0\0\0\x45\0\0\0\xb3\0\0\0\x56\0\0\0\x97\0\0\0\xce\0\0\0"

        "\x16\0\0\0\x7f\0\0\0\x03\0\0\0\xad\0\0\0\x6c\0\0\0\xb0\0\0\0\xf0\0\0\0\xac\0\0\0"
        "\x2d\0\0\0\xae\0\0\0\x79\0\0\0\x40\0\0\0\xda\0\0\0\x7b\0\0\0\xcc\0\0\0\x29\0\0\0"
        "\x44\0\0\0\xc1\0\0\0\x75\0\0\0\xa8\0\0\0\x78\0\0\0\x3a\0\0\0\x25\0\0\0\x7a\0\0\0"
        "\x8b\0\0\0\x48\0\0\0\x65\0\0\0\xec\0\0\0\x15\0\0\0\x83\0\0\0\x18\0\0\0\xb7\0\0\0"

        "\x60\0\0\0\x38\0\0\0\xfc\0\0\0\x73\0\0\0\x11\0\0\0\x68\0\0\0\xf3\0\0\0\x0b\0\0\0"
        "\xb9\0\0\0\x5c\0\0\0\xc0\0\0\0\x51\0\0\0\xa7\0\0\0\xc2\0\0\0\x0a\0\0\0\x13\0\0\0"
        "\x10\0\0\0\x82\0\0\0\x92\0\0\0\x7c\0\0\0\x77\0\0\0\xd0\0\0\0\x08\0\0\0\xaa\0\0\0"
        "\xb8\0\0\0\x9e\0\0\0\x6b\0\0\0\x17\0\0\0\x91\0\0\0\x00\0\0\0\x46\0\0\0\x28\0\0\0"

        "\x72\0\0\0\xb6\0\0\0\xf9\0\0\0\xe4\0\0\0\x84\0\0\0\x37\0\0\0\x49\0\0\0\x42\0\0\0"
        "\x2f\0\0\0\x3d\0\0\0\x4d\0\0\0\xa3\0\0\0\xc6\0\0\0\x80\0\0\0\x0e\0\0\0\x21\0\0\0"
        "\x9d\0\0\0\xdb\0\0\0\xf5\0\0\0\x89\0\0\0\xb4\0\0\0\xa6\0\0\0\x1b\0\0\0\xf8\0\0\0"
        "\x20\0\0\0\xa1\0\0\0\x52\0\0\0\xeb\0\0\0\xcd\0\0\0\x9c\0\0\0\xdd\0\0\0\x1c\0\0\0"

        "\xd2\0\0\0\xb1\0\0\0\xd9\0\0\0\xba\0\0\0\x02\0\0\0\x7d\0\0\0\x99\0\0\0\x62\0\0\0"
        "\xea\0\0\0\x5b\0\0\0\xa9\0\0\0\xc9\0\0\0\xbd\0\0\0\x74\0\0\0\xd3\0\0\0\xe9\0\0\0"
        "\xf6\0\0\0\x05\0\0\0\x1d\0\0\0\xfd\0\0\0\xfe\0\0\0\x1a\0\0\0\x4c\0\0\0\xd6\0\0\0"
        "\x33\0\0\0\xdf\0\0\0\xbf\0\0\0\xe2\0\0\0\x5e\0\0\0\xc4\0\0\0\x2a\0\0\0\xa2\0\0\0"

        "\xfa\0\0\0\x85\0\0\0\xff\0\0\0\xee\0\0\0\xc3\0\0\0\xd7\0\0\0\x66\0\0\0\x36\0\0\0"
        "\x4b\0\0\0\x35\0\0\0\xe5\0\0\0\x43\0\0\0\x1e\0\0\0\x76\0\0\0\xe6\0\0\0\xd4\0\0\0"
        "\xaf\0\0\0\x71\0\0\0\xd5\0\0\0\xf4\0\0\0\x30\0\0\0\x3f\0\0\0\x7e\0\0\0\x3c\0\0\0"
        "\x4f\0\0\0\xe7\0\0\0\x6a\0\0\0\xf1\0\0\0\x70\0\0\0\x26\0\0\0\xe3\0\0\0\x6e\0\0\0"

        "\x01\0\0\0\x61\0\0\0\xbb\0\0\0\x93\0\0\0\x34\0\0\0\x3b\0\0\0\x53\0\0\0\x14\0\0\0"
        "\xe8\0\0\0\xf2\0\0\0\xd8\0\0\0\xfb\0\0\0\xc8\0\0\0\xd1\0\0\0\x19\0\0\0\x41\0\0\0"
        "\x59\0\0\0\x69\0\0\0\x86\0\0\0\xbe\0\0\0\x63\0\0\0\x07\0\0\0\x55\0\0\0\x50\0\0\0"
        "\x5f\0\0\0\x4a\0\0\0\x58\0\0\0\x06\0\0\0\x31\0\0\0\xcf\0\0\0\xca\0\0\0\x88\0\0\0"

        "\xff\0\0\0\xff\0\0\0\xff\0\0\0\xff\0\0\0\x01\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0"
        "\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0"
        "\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0"
        "\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0"

        "\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0"
        "\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0"
        "\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0"
        "\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0\x00\0\0\0"
        , 1032);
    }
    else {
    memcpy((void*)(&decrypt.rc4_info), (void*)"\x00\x00\x00\x00\x00\x00\x00\x00\x64\x1f\x98\x12\xbc\x6d\xcb\xa4\x27\x47\x24\xe0\xab\x96\xf7\x0d\x4e\xa0\xb2\xdc\x32\x9b\x94\x90\x8d\xc5\x5a\x95\x2c\xef\x04\x3e\x9a\x39\x9f\xde\x0c\xc7\x09\xa5\x22\xed\xb5\x57\x67\x2b\x0f\x8f\x8a\xe1\x23\x2e\x87\x54\x5d\x8c\x6f\x81\x8e\x45\xb3\x56\x97\xce\x16\x7f\x03\xad\x6c\xb0\xf0\xac\x2d\xae\x79\x40\xda\x7b\xcc\x29\x44\xc1\x75\xa8\x78\x3a\x25\x7a\x8b\x48\x65\xec\x15\x83\x18\xb7\x60\x38\xfc\x73\x11\x68\xf3\x0b\xb9\x5c\xc0\x51\xa7\xc2\x0a\x13\x10\x82\x92\x7c\x77\xd0\x08\xaa\xb8\x9e\x6b\x17\x91\x00\x46\x28\x72\xb6\xf9\xe4\x84\x37\x49\x42\x2f\x3d\x4d\xa3\xc6\x80\x0e\x21\x9d\xdb\xf5\x89\xb4\xa6\x1b\xf8\x20\xa1\x52\xeb\xcd\x9c\xdd\x1c\xd2\xb1\xd9\xba\x02\x7d\x99\x62\xea\x5b\xa9\xc9\xbd\x74\xd3\xe9\xf6\x05\x1d\xfd\xfe\x1a\x4c\xd6\x33\xdf\xbf\xe2\x5e\xc4\x2a\xa2\xfa\x85\xff\xee\xc3\xd7\x66\x36\x4b\x35\xe5\x43\x1e\x76\xe6\xd4\xaf\x71\xd5\xf4\x30\x3f\x7e\x3c\x4f\xe7\x6a\xf1\x70\x26\xe3\x6e\x01\x61\xbb\x93\x34\x3b\x53\x14\xe8\xf2\xd8\xfb\xc8\xd1\x19\x41\x59\x69\x86\xbe\x63\x07\x55\x50\x5f\x4a\x58\x06\x31\xcf\xca\x88\xff\xff\xff\xff\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 1032);
    }
    decrypt.rc4_key_size=1;

    SecIn sec(stream, decrypt);
    BOOST_CHECK_EQUAL((uint32_t)(SEC_ENCRYPT|SEC_LOGON_INFO), (uint32_t)sec.flags);

    uint8_t * start_of_logon_info = stream.p;
    uint32_t codepage = stream.in_uint32_le();
    BOOST_CHECK_EQUAL((uint32_t)0, codepage);

    // INFO_ENABLEWINDOWSKEY  = 0x00000100
    // INFO_MAXIMIZESHELL     = 0x00000020
    // INFO_UNICODE           = 0x00000010
    // INFO_DISABLECTRLALTDEL = 0x00000002
    // INFO_MOUSE             = 0x00000001
    uint32_t flags = stream.in_uint32_le();
    BOOST_CHECK_EQUAL((uint32_t)0x133, flags);

    uint16_t cbDomain = stream.in_uint16_le();
    BOOST_CHECK_EQUAL((uint16_t)0, cbDomain);
    uint16_t cbUser = stream.in_uint16_le();
    BOOST_CHECK_EQUAL((uint16_t)10, cbUser);
    uint16_t cbPassword = stream.in_uint16_le();
    BOOST_CHECK_EQUAL((uint16_t)0, cbPassword);
    uint16_t cbAlternateShell = stream.in_uint16_le();
    BOOST_CHECK_EQUAL((uint16_t)0, cbAlternateShell);
    uint16_t cbWorkingDir = stream.in_uint16_le();
    BOOST_CHECK_EQUAL((uint16_t)0, cbWorkingDir);
    BOOST_CHECK(0 == memcmp("\x00\x00", stream.in_uint8p(cbDomain+2), cbDomain+2));
    BOOST_CHECK(0 == memcmp("k\x00r\x00i\x00s\x00s\x00\x00\x00", stream.in_uint8p(cbUser+2), cbUser+2));
    BOOST_CHECK(0 == memcmp("\x00\x00", stream.in_uint8p(cbPassword+2), cbPassword+2));
    BOOST_CHECK(0 == memcmp("\x00\x00", stream.in_uint8p(cbAlternateShell+2), cbAlternateShell+2));
    BOOST_CHECK(0 == memcmp("\x00\x00", stream.in_uint8p(cbWorkingDir+2), cbWorkingDir+2));

    uint16_t clientAddressFamily = stream.in_uint16_le();
    // 0x00002 AF_INET The clientAddress field contains an IPv4 address.
    // 0x0017 AF_INET6 The clientAddress field contains an IPv6 address.
    BOOST_CHECK_EQUAL((uint16_t)0x00002, clientAddressFamily);

    uint16_t cbClientAddress = stream.in_uint16_le();
    BOOST_CHECK_EQUAL((uint16_t)20, cbClientAddress);
    char * p = (char*)stream.in_uint8p(cbClientAddress);
    // 127.0.0.1
    char expectedClientAddress[] = {
        '1', 0, '2', 0, '7', 0, '.', 0, '0', 0, '.', 0, '0', 0, '.', 0, '1', 0, 0, 0};
    BOOST_CHECK(0 == memcmp(expectedClientAddress, p , cbClientAddress));

    uint16_t cbClientDir = stream.in_uint16_le();
    BOOST_CHECK_EQUAL((uint16_t)60, cbClientDir);

    char * p2 = (char*)stream.in_uint8p(cbClientDir);

    // C:\WINNT\System32\mstscax.dll
    char expectedClientDir[] = {
        0x43, 0, 0x3A, 0, 0x5C, 0, 0x57, 0, 0x49, 0,
        0x4e, 0, 0x4e, 0, 0x54, 0, 0x5c, 0, 0x53, 0,
        0x79, 0, 0x73, 0, 0x74, 0, 0x65, 0, 0x6d, 0,
        0x33, 0, 0x32, 0, 0x5C, 0, 0x6D, 0, 0x73, 0,
        0x74, 0, 0x73, 0, 0x63, 0, 0x61, 0, 0x78, 0,
        0x2E, 0, 0x64, 0, 0x6C, 0, 0x6C, 0, 0, 0,
    };

    BOOST_CHECK(0 == memcmp(expectedClientDir, p2 , cbClientDir));

// Bias (4 bytes): A 32-bit, unsigned integer that contains the current bias for
// local time translation on the client. The bias is the difference, in minutes,
// between Coordinated Universal Time (UTC) and local time. All translations
// between UTC and local time are based on the following formula:
// UTC = local time + bias

    // Note: really this is a signed integer as Bias can be negative!

    uint32_t Bias = stream.in_uint32_le();
    BOOST_CHECK_EQUAL((uint32_t)0xFFFFFFC4, Bias);
    BOOST_CHECK_EQUAL((uint32_t)-60, Bias);

// StandardName (64 bytes): An array of 32 Unicode characters. The descriptive
// name for standard time on the client.
    char * pStandardName = (char*)stream.in_uint8p(64);
    char StandardName[] = {
        'G', 0, 'T', 0, 'B', 0, ',', 0, ' ', 0,
        'n', 0, 'o', 0, 'r', 0, 'm', 0, 'a', 0,
        'l', 0, 't', 0, 'i', 0, 'd', 0,   0, 0,
          0, 0,   0, 0,   0, 0,   0, 0,   0, 0,
          0, 0,   0, 0,   0, 0,   0, 0,   0, 0,
          0, 0,   0, 0,   0, 0,   0, 0,   0, 0,
        0,   0, 0,   0,
    };

    BOOST_CHECK(0 == memcmp(StandardName, pStandardName, 64));

// StandardDate (16 bytes): A TS_SYSTEMTIME (section 2.2.1.11.1.1.1.1.1)
// structure that contains the date and local time when the transition from
// daylight saving time to standard time occurs on the client. If this field
// contains a valid date and time, then the DaylightDate field MUST also contain
// a valid date and time. If the wYear, wMonth, wDayOfWeek, wDay, wHour,
// wMinute, wSecond, and wMilliseconds fields are all set to zero, then the
// client does not support daylight saving time.
    uint16_t wYear = stream.in_uint16_le();
    BOOST_CHECK_EQUAL((uint16_t)0, wYear);
    uint16_t wMonth = stream.in_uint16_le();
    BOOST_CHECK_EQUAL((uint16_t)10, wMonth);
    uint16_t wDayOfWeek = stream.in_uint16_le();
    BOOST_CHECK_EQUAL((uint16_t)0, wDayOfWeek);
    uint16_t wDay = stream.in_uint16_le();
    BOOST_CHECK_EQUAL((uint16_t)5, wDay);
    uint16_t wHour = stream.in_uint16_le();
    BOOST_CHECK_EQUAL((uint16_t)3, wHour);
    uint16_t wMinutes = stream.in_uint16_le();
    BOOST_CHECK_EQUAL((uint16_t)0, wMinutes);
    uint16_t wSeconds = stream.in_uint16_le();
    BOOST_CHECK_EQUAL((uint16_t)0, wSeconds);
    uint16_t wMilliseconds = stream.in_uint16_le();
    BOOST_CHECK_EQUAL((uint16_t)0, wMilliseconds);

// StandardBias (4 bytes): A 32-bit, unsigned integer that contains the bias
// value to be used during local time translations that occur during standard
// time. This value is added to the value of the Bias field to form the bias
// used during standard time. This field MUST be ignored if a valid date and
// time is not specified in the StandardDate field or the wYear, wMonth,
// wDayOfWeek, wDay, wHour, wMinute, wSecond, and wMilliseconds fields of the
// StandardDate field are all set to zero.
    uint32_t StandardBias = stream.in_uint32_le();
    BOOST_CHECK_EQUAL((uint32_t)0, StandardBias);

// DaylightName (64 bytes): An array of 32 Unicode characters. The descriptive
// name for daylight saving time on the client.
    char * pDaylightName = (char*)stream.in_uint8p(64);
    char DaylightName[] = {
        'G', 0, 'T', 0, 'B', 0, ',', 0, ' ', 0,
        's', 0, 'o', 0, 'm', 0, 'm', 0, 'a', 0,
        'r', 0, 't', 0, 'i', 0, 'd', 0,   0, 0,
          0, 0,   0, 0,   0, 0,   0, 0,   0, 0,
          0, 0,   0, 0,   0, 0,   0, 0,   0, 0,
          0, 0,   0, 0,   0, 0,   0, 0,   0, 0,
        0,   0, 0,   0,
    };
    BOOST_CHECK(0 == memcmp(DaylightName, pDaylightName, 64));

// DaylightDate (16 bytes): A TS_SYSTEMTIME (section 2.2.1.11.1.1.1.1.1)
// structure that contains a date and local time when the transition from
// standard time to daylight saving time occurs on the client. If this field
// contains a valid date and time, then the StandardDate field MUST also contain
// a valid date and time. If the wYear, wMonth, wDayOfWeek, wDay, wHour,
// wMinute, wSecond, and wMilliseconds fields are all set to zero, then the
// client does not support daylight saving time.
    uint16_t wYearDD = stream.in_uint16_le();
    BOOST_CHECK_EQUAL((uint16_t)0, wYearDD);
    uint16_t wMonthDD = stream.in_uint16_le();
    BOOST_CHECK_EQUAL((uint16_t)3, wMonthDD);
    uint16_t wDayOfWeekDD = stream.in_uint16_le();
    BOOST_CHECK_EQUAL((uint16_t)0, wDayOfWeekDD);
    uint16_t wDayDD = stream.in_uint16_le();
    BOOST_CHECK_EQUAL((uint16_t)5, wDayDD);
    uint16_t wHourDD = stream.in_uint16_le();
    BOOST_CHECK_EQUAL((uint16_t)2, wHourDD);
    uint16_t wMinutesDD = stream.in_uint16_le();
    BOOST_CHECK_EQUAL((uint16_t)0, wMinutesDD);
    uint16_t wSecondsDD = stream.in_uint16_le();
    BOOST_CHECK_EQUAL((uint16_t)0, wSecondsDD);
    uint16_t wMillisecondsDD = stream.in_uint16_le();
    BOOST_CHECK_EQUAL((uint16_t)0, wMillisecondsDD);

// DaylightBias (4 bytes): A 32-bit, unsigned integer that contains the bias
// value to be used during local time translations that occur during daylight
// saving time. This value is added to the value of the Bias field to form the
// bias used during daylight saving time. This field MUST be ignored if a valid
// date and time is not specified in the DaylightDate field or the wYear,
// wMonth, wDayOfWeek, wDay, wHour, wMinute, wSecond, and wMilliseconds fields
// of the DaylightDate field are all set to zero.
    uint32_t DaylightBias = stream.in_uint32_le();
    BOOST_CHECK_EQUAL((uint32_t)0xFFFFFFC4, DaylightBias);
    BOOST_CHECK_EQUAL((uint32_t)-60, DaylightBias);

// clientSessionId (4 bytes): A 32-bit, unsigned integer. This field was added
// in RDP 5.1 and is currently ignored by the server. It SHOULD be set to 0.

    uint32_t clientSessionId = stream.in_uint32_le();
    BOOST_CHECK_EQUAL((uint32_t)0xFFFFFFFe, clientSessionId);


// performanceFlags (4 bytes): A 32-bit, unsigned integer. It specifies a list
// of server desktop shell features to enable or disable in the session (with
// the goal of optimizing bandwidth usage). It is used by RDP 5.1, 5.2, 6.0,
// 6.1, and 7.0 servers.

// +--------------------------------------------+------------------------------+
// | 0x00000001 PERF_DISABLE_WALLPAPER          | Disable desktop wallpaper.   |
// +--------------------------------------------+------------------------------+
// | 0x00000002 PERF_DISABLE_FULLWINDOWDRAG     | Disable full-window drag     |
// |                                            |(only the window outline is   |
// |                                            |displayed when the window is  |
// |                                            | moved).                      |
// +--------------------------------------------+------------------------------+
// | 0x00000004 PERF_DISABLE_MENUANIMATIONS     | Disable menu animations.     |
// +--------------------------------------------+------------------------------+
// | 0x00000008 PERF_DISABLE_THEMING            | Disable user interface theme.|
// +--------------------------------------------+------------------------------+
// | 0x00000010 PERF_RESERVED1                  | Reserved for future use.     |
// +--------------------------------------------+------------------------------+
// | 0x00000020 PERF_DISABLE_CURSOR_SHADOW      | Disable mouse cursor shadows.|
// +--------------------------------------------+------------------------------+
// | 0x00000040 PERF_DISABLE_CURSORSETTINGS     | Disable cursor blinking.     |
// +--------------------------------------------+------------------------------+
// | 0x00000080 PERF_ENABLE_FONT_SMOOTHING      | Enable font smoothing.       |
// +--------------------------------------------+------------------------------+
// | 0x00000100 PERF_ENABLE_DESKTOP_COMPOSITION | Enable Desktop Composition.  |
// +--------------------------------------------+------------------------------+
// | 0x80000000 PERF_RESERVED2                  | Reserved for future use.     |
// +--------------------------------------------+------------------------------+

    // 0x00000001 PERF_DISABLE_WALLPAPER
    // 0x00000002 PERF_DISABLE_FULLWINDOWDRAG
    // 0x00000004 PERF_DISABLE_MENUANIMATIONS
    uint32_t performanceFlags = stream.in_uint32_le();
    BOOST_CHECK_EQUAL((uint32_t)7, performanceFlags);

// cbAutoReconnectLen (2 bytes): A 16-bit, unsigned integer. The size in bytes
// of the cookie specified by the autoReconnectCookie field. This field is only
// read by RDP 5.2, 6.0, 6.1, and 7.0 servers.

    uint16_t cbAutoReconnectLen = stream.in_uint16_le();
    BOOST_CHECK_EQUAL((uint16_t)0, cbAutoReconnectLen);

// autoReconnectCookie (28 bytes): Buffer containing an ARC_CS_PRIVATE_PACKET
// structure (section 2.2.4.3). This buffer is a unique cookie that allows a
// disconnected client to seamlessly reconnect to a previously established
// session (see section 5.5 for more details). The autoReconnectCookie field is
// only read by RDP 5.2, 6.0, 6.1, and 7.0 servers and the maximum allowed
// length is 128 bytes.
    stream.skip_uint8(cbAutoReconnectLen);

// reserved1 (2 bytes): This field is reserved for future use and has no affect
// on RDP wire traffic. If this field is present, the reserved2 field MUST
// be present.
    stream.skip_uint8(2);

// reserved2 (2 bytes): This field is reserved for future use and has no affect
// on RDP wire traffic. This field MUST be present if the reserved1 field
// is present.
    stream.skip_uint8(2);

// Rdesktop advertise an overly large buffer, but this is not a problem
    BOOST_CHECK((uint32_t)318 >= (uint32_t)(stream.p - start_of_logon_info));

}
