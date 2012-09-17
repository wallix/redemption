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

   Licence Management

*/

#if !defined(__CORE_RDP_LIC_HPP__)
#define __CORE_RDP_LIC_HPP__

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "RDP/sec.hpp"

namespace LIC 
{
    enum {
        LICENSE_TOKEN_SIZE             = 10,
        LICENSE_HWID_SIZE              = 20,
        LICENSE_SIGNATURE_SIZE         = 16,
    };

    enum {
        LICENSE_REQUEST             = 0x01,
        PLATFORM_CHALLENGE          = 0x02,
        NEW_LICENSE                 = 0x03,
        UPGRADE_LICENSE             = 0x04,
        LICENSE_INFO                = 0x12,
        NEW_LICENSE_REQUEST         = 0x13,
        PLATFORM_CHALLENGE_RESPONSE = 0x15,
        ERROR_ALERT                 = 0xff
    };

    enum {
        LICENSE_TAG_USER            = 0x000f,
        LICENSE_TAG_HOST            = 0x0010,
    };


    // +------------------------------------+-------------------------------------+
    // | 0x0001 BB_DATA_BLOB                | Used by License Information PDU and |
    // |                                    | Platform Challenge Response PDU     |
    // |                                    | ([MS-RDPELE] sections 2.2.2.3 and   |
    // |                                    | 2.2.2.5).                           |
    // +------------------------------------+-------------------------------------+
    // | 0x0002 BB_RANDOM_BLOB              | Used by License Information PDU and |
    // |                                    | New License Request PDU ([MS-RDPELE]|
    // |                                    | sections 2.2.2.3 and 2.2.2.2).      |
    // +------------------------------------+-------------------------------------+
    // | 0x0003 BB_CERTIFICATE_BLOB         | Used by License Request PDU         |
    // |                                    | ([MS-RDPELE] section 2.2.2.1).      |
    // +------------------------------------+-------------------------------------+
    // | 0x0004 BB_ERROR_BLOB               | Used by License Error PDU (section  |
    // |                                    | 2.2.1.12).                          |
    // +------------------------------------+-------------------------------------+
    // | 0x0009 BB_ENCRYPTED_DATA_BLOB      | Used by Platform Challenge Response |
    // |                                    | PDU and Upgrade License PDU         |
    // |                                    | ([MS-RDPELE] sections 2.2.2.5 and   |
    // |                                    | 2.2.2.6).                           |
    // +------------------------------------+-------------------------------------+
    // | 0x000D BB_KEY_EXCHG_ALG_BLOB       | Used by License Request PDU         |
    // |                                    | ([MS-RDPELE] section 2.2.2.1).      |
    // +------------------------------------+-------------------------------------+
    // | 0x000E BB_SCOPE_BLOB               | Used by License Request PDU         |
    // |                                    | ([MS-RDPELE] section 2.2.2.1).      |
    // +------------------------------------+-------------------------------------+
    // | 0x000F BB_CLIENT_USER_NAME_BLOB    | Used by New License Request PDU     |
    // |                                    | ([MS-RDPELE] section 2.2.2.2).      |
    // +------------------------------------+-------------------------------------+
    // | 0x0010 BB_CLIENT_MACHINE_NAME_BLOB | Used by New License Request PDU     |
    // |                                    | ([MS-RDPELE] section 2.2.2.2).      |
    // +------------------------------------+-------------------------------------+

    enum {
        BB_DATA_BLOB                = 0x0001,
        BB_RANDOM_BLOB              = 0x0002,
        BB_CERTIFICATE_BLOB         = 0x0003,
        BB_ERROR_BLOB               = 0x0004,
        BB_ENCRYPTED_DATA_BLOB      = 0x0009,
        BB_KEY_EXCHG_ALG_BLOB       = 0x000D,
        BB_SCOPE_BLOB               = 0x000E,
        BB_CLIENT_USER_NAME_BLOB    = 0x000F,
        BB_CLIENT_MACHINE_NAME_BLOB = 0x0010,
    };

    enum {
        KEY_EXCHANGE_ALG_RSA        = 0x01,
    };

    // 2.2.1.12.1.1 Licensing Preamble (LICENSE_PREAMBLE)
    // --------------------------------------------------

    // Note: Some of the information in this section is subject to
    // change because it applies to a preliminary implementation of the
    // protocol or structure. For information about specific differences
    // between versions, see the behavior notes that are provided in the
    // Product Behavior appendix.

    // The LICENSE_PREAMBLE structure precedes every licensing packet
    // sent on the wire.

    // bMsgType (1 byte): An 8-bit, unsigned integer. A type of the
    // licensing packet. For more details about the different licensing
    // packets, see [MS-RDPELE] section 2.2.2.

    // Sent by server:
    // 0x01 LICENSE_REQUEST Indicates a License Request PDU ([MS-RDPELE] section 2.2.2.1).
    // 0x02 PLATFORM_CHALLENGE Indicates a Platform Challenge PDU ([MS-RDPELE] section 2.2.2.4).
    // 0x03 NEW_LICENSE Indicates a New License PDU ([MS-RDPELE] section 2.2.2.7).
    // 0x04 UPGRADE_LICENSE Indicates an Upgrade License PDU ([MS-RDPELE] section 2.2.2.6).

    // Sent by client:
    // 0x12 LICENSE_INFO Indicates a License Information PDU ([MS-RDPELE] section 2.2.2.3).
    // 0x13 NEW_LICENSE_REQUEST Indicates a New License Request PDU ([MS-RDPELE] section 2.2.2.2).
    // 0x15 PLATFORM_CHALLENGE_RESPONSE Indicates a Platform Challenge Response PDU ([MS-RDPELE] section 2.2.2.5).

    // Sent by either client or server:
    // 0xFF ERROR_ALERT Indicates a Licensing Error Message PDU (section 2.2.1.12.1.3).

    // flags (1 byte): An 8-bit unsigned integer. License preamble flags.

    // +-----------------------------------+------------------------------------------------------+
    // | 0x0F LicenseProtocolVersionMask   | The license protocol version. See the discussion     |
    // |                                   | which follows this table for more information.       |
    // +-----------------------------------+------------------------------------------------------+
    // | 0x80 EXTENDED_ERROR_MSG_SUPPORTED | Indicates that extended error information using the  |
    // |                                   | License Error Message (section 2.2.1.12.1.3) is      |
    // |                                   | supported.                                           |
    // +-----------------------------------+------------------------------------------------------+

    // The LicenseProtocolVersionMask is a 4-bit value containing the supported license protocol version. The following are possible version values.
    // +--------------------------+------------------------------------------------+
    // | 0x2 PREAMBLE_VERSION_2_0 | RDP 4.0                                        |
    // +--------------------------+------------------------------------------------+
    // | 0x3 PREAMBLE_VERSION_3_0 | RDP 5.0, 5.1, 5.2, 6.0, 6.1, 7.0, 7.1, and 8.0 |
    // +--------------------------+------------------------------------------------+


    // wMsgSize (2 bytes): An 16-bit, unsigned integer. The size in
    // bytes of the licensing packet (including the size of the preamble).
    // -------------------------------------------------------------------

    struct RecvFactory
    {
        uint8_t tag;
        uint8_t flags;
        uint16_t wMsgSize;
        RecvFactory(Stream & stream)
        {
            if (!stream.check_rem(4)){
                LOG(LOG_ERR, "Not enough data to read licence info header, need %u, got %u", 4, stream.end - stream.p);
                throw Error(ERR_LIC);
            }
            this->tag = stream.in_uint8();
            this->flags = stream.in_uint8();
            this->wMsgSize = stream.in_uint16_le();
            if (this->wMsgSize > stream.size()){
                LOG(LOG_ERR, "Not enough data to read licence data, need %u, got %u", 4, this->wMsgSize, stream.size());
                throw Error(ERR_LIC);
            }
            stream.p -= 4;
        }
    };

    // 2.2.2.1 Server License Request (SERVER_LICENSE_REQUEST)
    // =======================================================

    // The Server License Request packet is sent to the client to initiate the RDP licensing handshake.

    // ServerRandom (32 bytes): A 32-byte array containing a random number. This random
    //  number is created using a cryptographically secure pseudo-random number generator and is
    //  used to generate licensing encryption keys (see section 5.1.3). These keys are used to
    //  encrypt licensing data in subsequent licensing messages (see sections 5.1.4 and 5.1.5).

    // ProductInfo (variable): A variable-length Product Information structure. This structure
    //  contains the details of the product license required for connecting to the terminal server.

    // 2.2.2.1.1 Product Information (PRODUCT_INFO)
    // ============================================
    // The Product Information packet contains the details of the product license that is required for
    // connecting to the terminal server. The client uses this structure together with the scope list to
    // search for and identify an appropriate license in its license store. Depending on the outcome of the
    // search, the client sends a Client New License Request (section 2.2.2.2), Client License Information
    // packet (section 2.2.2.3), or license error message (section 2.2.2.7.1) to the server.

    // ProductInfo::dwVersion (4 bytes): A 32-bit unsigned integer that contains the license version information.
    // The high-order word contains the major version of the operating system on which the terminal
    // server is running, while the low-order word contains the minor version.<6>

    // ProductInfo::cbCompanyName (4 bytes): An unsigned 32-bit integer that contains the number of bytes in
    // the pbCompanyName field, including the terminating null character. This value MUST be
    // greater than zero.

    // ProductInfo::pbCompanyName (variable): Contains a null-terminated Unicode string that specifies the
    // company name.<7>

    // ProductInfo::cbProductId (4 bytes): An unsigned 32-bit integer that contains the number of bytes in the
    // pbProductId field, including the terminating null character. This value MUST be greater than
    // zero.

    // ProductInfo::pbProductId (variable): Contains a null-terminated Unicode string that identifies the type of
    // the license that is required by the terminal server. It MAY have the following string value. 
    // "A02" Per device or per user license
    // ------------------------------------------------------------------

    // KeyExchangeList (variable): A Licensing Binary BLOB structure (see [MS-RDPBCGR] section
    //  2.2.1.12.1.2) of type BB_KEY_EXCHG_ALG_BLOB (0x000D). This BLOB contains the list of 32-
    //  bit unsigned integers specifying key exchange algorithms that the server supports. The
    //  terminal server supports only one key exchange algorithm as of now, so the BLOB contains
    //  the following value.

    // 0x00000001 KEY_EXCHANGE_ALG_RSA Indicates RSA key exchange algorithm with a 512-bit asymmetric key.<3>

    // ServerCertificate (variable): A Licensing Binary BLOB structure (see [MS-RDPBCGR] section
    //  2.2.1.12.1.2) of type BB_CERTIFICATE_BLOB (0x0003). This BLOB contains the terminal
    //  server certificate (see section 2.2.1.4). The terminal server can choose not to send the
    //  certificate by setting the wblobLen field in the Licensing Binary BLOB structure to 0. If
    //  encryption is in effect and is already protecting RDP traffic, the licensing protocol MAY<4>
    //  choose not to send the server certificate (for RDP security measures, see [MS-RDPBCGR]
    //  sections 5.3 and 5.4). If the licensing protocol chooses not to send the server certificate, then
    //  the client uses the public key obtained from the server certificate sent as part of Server
    //  Security Data in the Server MCS Connect Response PDU (see [MS-RDPBCGR] section 2.2.1.4).

    // ScopeList (variable): A variable-length Scope List structure that contains a list of entities that
    //  issued the client license. This list is used by the client in conjunction with ProductInfo to
    //  search for an appropriate license in its license store.<5>

    // 2.2.2.1.2 Scope List (SCOPE_LIST)
    // =================================
    // The Scope List packet contains a list of entities that issued a client license. The client uses the name
    // of the issuers in the Scope structures of this list in conjunction with the Product Information
    // structure to search the license store for a matching client license.

    // ScopeList::ScopeCount (4 bytes): A 32-bit unsigned integer containing the number of elements in the ScopeArray field.

    // ScopeList::ScopeArray (variable): An array of Scope structures containing ScopeCount elements. <8>

    // 2.2.2.1.2.1 Scope (SCOPE)
    // =========================
    // The Scope packet contains the name of an entity that issued a client license.

    // Scope (variable): A Licensing Binary BLOB structure (see [MS-RDPBCGR] section 2.2.1.12.1.2)
    // of type BB_SCOPE_BLOB (0x000E). This BLOB contains the name of a license issuer in null-
    // terminated ANSI characters, as specified in [ISO/IEC-8859-1], string format, with an
    // implementation-specific valid code page.

    // ------------------------------------------------------------------

    // +------------------------------------+-------------------------------------+
    // | 0x0001 BB_DATA_BLOB                | Used by License Information PDU and |
    // |                                    | Platform Challenge Response PDU     |
    // |                                    | ([MS-RDPELE] sections 2.2.2.3 and   |
    // |                                    | 2.2.2.5).                           |
    // +------------------------------------+-------------------------------------+
    // | 0x0002 BB_RANDOM_BLOB              | Used by License Information PDU and |
    // |                                    | New License Request PDU ([MS-RDPELE]|
    // |                                    | sections 2.2.2.3 and 2.2.2.2).      |
    // +------------------------------------+-------------------------------------+
    // | 0x0003 BB_CERTIFICATE_BLOB         | Used by License Request PDU         |
    // |                                    | ([MS-RDPELE] section 2.2.2.1).      |
    // +------------------------------------+-------------------------------------+
    // | 0x0004 BB_ERROR_BLOB               | Used by License Error PDU (section  |
    // |                                    | 2.2.1.12).                          |
    // +------------------------------------+-------------------------------------+
    // | 0x0009 BB_ENCRYPTED_DATA_BLOB      | Used by Platform Challenge Response |
    // |                                    | PDU and Upgrade License PDU         |
    // |                                    | ([MS-RDPELE] sections 2.2.2.5 and   |
    // |                                    | 2.2.2.6).                           |
    // +------------------------------------+-------------------------------------+
    // | 0x000D BB_KEY_EXCHG_ALG_BLOB       | Used by License Request PDU         |
    // |                                    | ([MS-RDPELE] section 2.2.2.1).      |
    // +------------------------------------+-------------------------------------+
    // | 0x000E BB_SCOPE_BLOB               | Used by License Request PDU         |
    // |                                    | ([MS-RDPELE] section 2.2.2.1).      |
    // +------------------------------------+-------------------------------------+
    // | 0x000F BB_CLIENT_USER_NAME_BLOB    | Used by New License Request PDU     |
    // |                                    | ([MS-RDPELE] section 2.2.2.2).      |
    // +------------------------------------+-------------------------------------+
    // | 0x0010 BB_CLIENT_MACHINE_NAME_BLOB | Used by New License Request PDU     |
    // |                                    | ([MS-RDPELE] section 2.2.2.2).      |
    // +------------------------------------+-------------------------------------+

    // 4.1 SERVER LICENCE REQUEST EXCERPT 
    // ----------------------------------

    //    The Server License Request message is the first message sent by the server
    // as part of the licensing protocol. See sections 2.2.2.1 and 3.2.5.1.

    //    00000000  01 03 98 08 84 ef ae 20-b1 d5 9e 36 49 1a e8 2e   ....... ...6I...
    //    00000010  0a 99 89 ac 49 a6 47 4f-33 9b 5a b9 95 03 a6 c6   ....I.GO3.Z.....
    //    00000020  c2 3c 3f 61 00 00 06 00-2c 00 00 00 4d 00 69 00   .<?a....,...M.i.
    //    00000030  63 00 72 00 6f 00 73 00-6f 00 66 00 74 00 20 00   c.r.o.s.o.f.t. .
    //    00000040  43 00 6f 00 72 00 70 00-6f 00 72 00 61 00 74 00   C.o.r.p.o.r.a.t.
    //    00000050  69 00 6f 00 6e 00 00 00-08 00 00 00 41 00 30 00   i.o.n.......A.0.
    //    00000060  32 00 00 00 0d 00 04 00-01 00 00 00 03 00 12 08   2...............
    //    00000070  02 00 00 80 02 00 00 00-f5 02 00 00 30 82 02 f1   ............0...
    //    00000080  30 82 01 dd a0 03 02 01-02 02 08 01 9e 24 a2 f2   0............$..
    //    00000090  ae 90 80 30 09 06 05 2b-0e 03 02 1d 05 00 30 32   ...0...+......02
    //    000000A0  31 30 30 13 06 03 55 04-03 1e 0c 00 52 00 4f 00   100...U.....R.O.
    //    000000B0  44 00 45 00 4e 00 54 30-19 06 03 55 04 07 1e 12   D.E.N.T0...U....
    //    000000C0  00 57 00 4f 00 52 00 4b-00 47 00 52 00 4f 00 55   .W.O.R.K.G.R.O.U
    //    000000D0  00 50 30 1e 17 0d 37 30-30 35 32 37 30 31 31 31   .P0...7005270111
    //    000000E0  30 33 5a 17 0d 34 39 30-35 32 37 30 31 31 31 30   03Z..49052701110
    //    000000F0  33 5a 30 32 31 30 30 13-06 03 55 04 03 1e 0c 00   3Z02100...U.....
    //    00000100  52 00 4f 00 44 00 45 00-4e 00 54 30 19 06 03 55   R.O.D.E.N.T0...U
    //    00000110  04 07 1e 12 00 57 00 4f-00 52 00 4b 00 47 00 52   .....W.O.R.K.G.R
    //    00000120  00 4f 00 55 00 50 30 82-01 22 30 0d 06 09 2a 86   .O.U.P0.."0...*.
    //    00000130  48 86 f7 0d 01 01 01 05-00 03 82 01 0f 00 30 82   H.............0.
    //    00000140  01 0a 02 82 01 01 00 88-ad 7c 8f 8b 82 76 5a bd   .........|...vZ.
    //    00000150  8f 6f 62 18 e1 d9 aa 41-fd ed 68 01 c6 34 35 b0   .ob....A..h..45.
    //    00000160  29 04 ca 4a 4a 1c 7e 80-14 f7 8e 77 b8 25 ff 16   )..JJ.~....w.%..
    //    00000170  47 6f bd e2 34 3d 2e 02-b9 53 e4 33 75 ad 73 28   Go..4=...S.3u.s(
    //    00000180  80 a0 4d fc 6c c0 22 53-1b 2c f8 f5 01 60 19 7e   ..M.l."S.,...`.~
    //    00000190  79 19 39 8d b5 ce 39 58-dd 55 24 3b 55 7b 43 c1   y.9...9X.U$;U{C.
    //    000001A0  7f 14 2f b0 64 3a 54 95-2b 88 49 0c 61 2d ac f8   ../.d:T.+.I.a-..
    //    000001B0  45 f5 da 88 18 5f ae 42-f8 75 c7 26 6d b5 bb 39   E...._.B.u.&m..9
    //    000001C0  6f cc 55 1b 32 11 38 8d-e4 e9 44 84 11 36 a2 61   o.U.2.8...D..6.a
    //    000001D0  76 aa 4c b4 e3 55 0f e4-77 8e de e3 a9 ea b7 41   v.L..U..w......A
    //    000001E0  94 00 58 aa c9 34 a2 98-c6 01 1a 76 14 01 a8 dc   ..X..4.....v....
    //    000001F0  30 7c 77 5a 20 71 5a a2-3f af 13 7e e8 fd 84 a2   0|wZ qZ.?..~....
    //    00000200  5b cf 25 e9 c7 8f a8 f2-8b 84 c7 04 5e 53 73 4e   [.%.........^SsN
    //    00000210  0e 89 a3 3c e7 68 5c 24-b7 80 53 3c 54 c8 c1 53   ...<.h\$..S<T..S
    //    00000220  aa 71 71 3d 36 15 d6 6a-9d 7d de ae f9 e6 af 57   .qq=6..j.}.....W
    //    00000230  ae b9 01 96 5d e0 4d cd-ed c8 d7 f3 01 03 38 10   ....].M.......8.
    //    00000240  be 7c 42 67 01 a7 23 02-03 01 00 01 a3 13 30 11   .|Bg..#.......0.
    //    00000250  30 0f 06 03 55 1d 13 04-08 30 06 01 01 ff 02 01   0...U....0......
    //    00000260  00 30 09 06 05 2b 0e 03-02 1d 05 00 03 82 01 01   .0...+..........
    //    00000270  00 81 dd d2 d3 33 d4 a3-b6 8e 6e 7d 9f fd 73 9f   .....3....n}..s.
    //    00000280  31 0b dd 42 82 3f 7e 21-df 28 cc 59 ca 6a c0 a9   1..B.?~!.(.Y.j..
    //    00000290  3d 30 7d e1 91 db 77 6b-8b 10 e6 fd bc 3c a3 58   =0}...wk.....<.X
    //    000002A0  48 c2 36 dd a0 0b f5 8e-13 da 7b 04 08 44 b4 f2   H.6.......{..D..
    //    000002B0  a8 0d 1e 0b 1d 1a 3f f9-9b 4b 5a 54 c5 b3 b4 03   ......?..KZT....
    //    000002C0  93 75 b3 72 5c 3d cf 63-0f 15 e1 64 58 de 52 8d   .u.r\=.c...dX.R.
    //    000002D0  97 79 0e a4 34 d5 66 05-58 b8 6e 79 b2 09 86 d5   .y..4.f.X.ny....
    //    000002E0  f0 ed c4 6b 4c ab 02 b8-16 5f 3b ed 88 5f d1 de   ...kL...._;.._..
    //    000002F0  44 e3 73 47 21 f7 03 ce-e1 6d 10 0f 95 cf 7c a2   D.sG!....m....|.
    //    00000300  7a a6 bf 20 db e1 93 04-c8 5e 6a be c8 01 5d 27   z.. .....^j...]'
    //    00000310  b2 03 0f 66 75 e7 cb ea-8d 4e 98 9d 22 ed 28 40   ...fu....N..".(@
    //    00000320  d2 7d a4 4b ef cc bf 01-2a 6d 3a 3e be 47 38 f8   .}.K....*m:>.G8.
    //    00000330  ea a4 c6 30 1d 5e 25 cf-fb e8 3d 42 dd 29 e8 99   ...0.^%...=B.)..
    //    00000340  89 9e bf 39 ee 77 09 d9-3e 8b 52 36 b6 bb 8b bd   ...9.w..>.R6....
    //    00000350  0d b2 52 aa 2c cf 38 4e-4d cf 1d 6d 5d 25 17 ac   ..R.,.8NM..m]%..
    //    00000360  2c f6 f0 65 5a c9 fe 31-53 b4 f0 0c 94 4e 0d 54   ,..eZ..1S....N.T
    //    00000370  8e fd 04 00 00 30 82 04-f9 30 82 03 e5 a0 03 02   .....0...0......
    //    00000380  01 02 02 05 01 00 00 00-02 30 09 06 05 2b 0e 03   .........0...+..
    //    00000390  02 1d 05 00 30 32 31 30-30 13 06 03 55 04 03 1e   ....02100...U...
    //    000003A0  0c 00 52 00 4f 00 44 00-45 00 4e 00 54 30 19 06   ..R.O.D.E.N.T0..
    //    000003B0  03 55 04 07 1e 12 00 57-00 4f 00 52 00 4b 00 47   .U.....W.O.R.K.G
    //    000003C0  00 52 00 4f 00 55 00 50-30 1e 17 0d 30 37 30 35   .R.O.U.P0...0705
    //    000003D0  32 36 31 32 34 35 35 33-5a 17 0d 33 38 30 31 31   26124553Z..38011
    //    000003E0  39 30 33 31 34 30 37 5a-30 81 92 31 81 8f 30 23   9031407Z0..1..0#
    //    000003F0  06 03 55 04 03 1e 1c 00-6e 00 63 00 61 00 6c 00   ..U.....n.c.a.l.
    //    00000400  72 00 70 00 63 00 3a 00-52 00 4f 00 44 00 45 00   r.p.c.:.R.O.D.E.
    //    00000410  4e 00 54 30 23 06 03 55-04 07 1e 1c 00 6e 00 63   N.T0#..U.....n.c
    //    00000420  00 61 00 6c 00 72 00 70-00 63 00 3a 00 52 00 4f   .a.l.r.p.c.:.R.O
    //    00000430  00 44 00 45 00 4e 00 54-30 43 06 03 55 04 05 1e   .D.E.N.T0C..U...
    //    00000440  3c 00 31 00 42 00 63 00-4b 00 65 00 62 00 68 00   <.1.B.c.K.e.b.h.
    //    00000450  70 00 58 00 5a 00 74 00-4c 00 71 00 4f 00 37 00   p.X.Z.t.L.q.O.7.
    //    00000460  53 00 51 00 6e 00 42 00-70 00 52 00 66 00 75 00   S.Q.n.B.p.R.f.u.
    //    00000470  64 00 64 00 64 00 59 00-3d 00 0d 00 0a 30 82 01   d.d.d.Y.=....0..
    //    00000480  1e 30 09 06 05 2b 0e 03-02 0f 05 00 03 82 01 0f   .0...+..........
    //    00000490  00 30 82 01 0a 02 82 01-01 00 c8 90 6b f0 c6 58   .0..........k..X
    //    000004A0  81 a6 89 1c 0e f2 f6 d9-82 12 71 a5 6e 51 db e0   ..........q.nQ..
    //    000004B0  32 66 aa 91 77 0e 88 ab-44 b7 d3 97 da 78 8f 0e   2f..w...D....x..
    //    000004C0  44 26 46 7f 16 d4 c6 63-eb ca 55 e5 4e 8b 2d a6   D&F....c..U.N.-.
    //    000004D0  6d 83 95 a7 a8 6a fa d0-be 26 80 ae ab 0a 64 90   m....j...&....d.
    //    000004E0  32 8c df 5c f8 f9 d0 7e-d1 6b 3a 29 7e 7d bd 02   2..\...~.k:)~}..
    //    000004F0  a3 86 6c fd a5 35 71 da-21 b4 ee a4 97 f3 a8 b2   ..l..5q.!.......
    //    00000500  12 db a4 27 57 36 c9 08-22 5c 54 f7 99 7b a3 2f   ...'W6.."\T..{./
    //    00000510  b8 5c d5 16 b8 19 27 6b-71 97 14 5b e8 1f 23 e8   .\....'kq..[..#.
    //    00000520  5c b8 1b 73 4b 6e 7a 03-13 ff 97 e9 62 b9 4a a0   \..sKnz.....b.J.
    //    00000530  51 23 c3 6c 32 3e 02 f2-63 97 23 1c c5 78 d8 fc   Q#.l2>..c.#..x..
    //    00000540  b7 07 4b b0 56 0f 74 df-c5 56 28 e4 96 fd 20 8e   ..K.V.t..V(... .
    //    00000550  65 5a e6 45 ed c1 05 3e-ab 58 55 40 af e2 47 a0   eZ.E...>.XU@..G.
    //    00000560  4c 49 a3 8d 39 e3 66 5f-93 33 6d f8 5f c5 54 e5   LI..9.f_.3m._.T.
    //    00000570  fb 57 3a de 45 12 b5 c7-05 4b 88 1f b4 35 0f 7c   .W:.E....K...5.|
    //    00000580  c0 75 17 c6 67 dd 48 80-cb 0a be 9d f6 93 60 65   .u..g.H.......`e
    //    00000590  34 eb 97 af 65 6d df bf-6f 5b 02 03 01 00 01 a3   4...em..o[......
    //    000005A0  82 01 bf 30 82 01 bb 30-14 06 09 2b 06 01 04 01   ...0...0...+....
    //    000005B0  82 37 12 04 01 01 ff 04-04 01 00 05 00 30 3c 06   .7...........0<.
    //    000005C0  09 2b 06 01 04 01 82 37-12 02 01 01 ff 04 2c 4d   .+.....7......,M
    //    000005D0  00 69 00 63 00 72 00 6f-00 73 00 6f 00 66 00 74   .i.c.r.o.s.o.f.t
    //    000005E0  00 20 00 43 00 6f 00 72-00 70 00 6f 00 72 00 61   . .C.o.r.p.o.r.a
    //    000005F0  00 74 00 69 00 6f 00 6e-00 00 00 30 81 cd 06 09   .t.i.o.n...0....
    //    00000600  2b 06 01 04 01 82 37 12-05 01 01 ff 04 81 bc 00   +.....7.........
    //    00000610  30 00 00 01 00 00 00 02-00 00 00 09 04 00 00 1c   0...............
    //    00000620  00 4a 00 66 00 4a 00 b0-00 01 00 33 00 64 00 32   .J.f.J.....3.d.2
    //    00000630  00 36 00 37 00 39 00 35-00 34 00 2d 00 65 00 65   .6.7.9.5.4.-.e.e
    //    00000640  00 62 00 37 00 2d 00 31-00 31 00 64 00 31 00 2d   .b.7.-.1.1.d.1.-
    //    00000650  00 62 00 39 00 34 00 65-00 2d 00 30 00 30 00 63   .b.9.4.e.-.0.0.c
    //    00000660  00 30 00 34 00 66 00 61-00 33 00 30 00 38 00 30   .0.4.f.a.3.0.8.0
    //    00000670  00 64 00 00 00 33 00 64-00 32 00 36 00 37 00 39   .d...3.d.2.6.7.9
    //    00000680  00 35 00 34 00 2d 00 65-00 65 00 62 00 37 00 2d   .5.4.-.e.e.b.7.-
    //    00000690  00 31 00 31 00 64 00 31-00 2d 00 62 00 39 00 34   .1.1.d.1.-.b.9.4
    //    000006A0  00 65 00 2d 00 30 00 30-00 63 00 30 00 34 00 66   .e.-.0.0.c.0.4.f
    //    000006B0  00 61 00 33 00 30 00 38-00 30 00 64 00 00 00 00   .a.3.0.8.0.d....
    //    000006C0  00 00 10 00 80 64 00 00-00 00 00 30 6e 06 09 2b   .....d.....0n..+
    //    000006D0  06 01 04 01 82 37 12 06-01 01 ff 04 5e 00 30 00   .....7......^.0.
    //    000006E0  00 00 00 0e 00 3e 00 52-00 4f 00 44 00 45 00 4e   .....>.R.O.D.E.N
    //    000006F0  00 54 00 00 00 37 00 38-00 34 00 34 00 30 00 2d   .T...7.8.4.4.0.-
    //    00000700  00 30 00 30 00 36 00 2d-00 35 00 38 00 36 00 37   .0.0.6.-.5.8.6.7
    //    00000710  00 30 00 34 00 35 00 2d-00 37 00 30 00 33 00 34   .0.4.5.-.7.0.3.4
    //    00000720  00 37 00 00 00 57 00 4f-00 52 00 4b 00 47 00 52   .7...W.O.R.K.G.R
    //    00000730  00 4f 00 55 00 50 00 00-00 00 00 30 25 06 03 55   .O.U.P.....0%..U
    //    00000740  1d 23 01 01 ff 04 1b 30-19 a1 10 a4 0e 52 00 4f   .#.....0.....R.O
    //    00000750  00 44 00 45 00 4e 00 54-00 00 00 82 05 01 00 00   .D.E.N.T........
    //    00000760  00 02 30 09 06 05 2b 0e-03 02 1d 05 00 03 82 01   ..0...+.........
    //    00000770  01 00 2e eb c7 0d b8 1d-47 11 9d 09 88 9b 51 dc   ........G.....Q.
    //    00000780  45 dd 56 51 e2 d1 23 11-39 9b 2d da c7 fe 7a d7   E.VQ..#.9.-...z.
    //    00000790  84 e3 3d 54 77 97 4d 19-92 30 64 a0 47 c6 2f 6d   ..=Tw.M..0d.G./m
    //    000007A0  93 d2 64 7c 76 c8 26 45-ad 5a 44 54 ea f6 4b 28   ..d|v.&E.ZDT..K(
    //    000007B0  77 1f 77 ea ec 74 02 38-68 9e 79 14 72 83 34 74   w.w..t.8h.y.r.4t
    //    000007C0  62 d2 c1 0c a4 0b f2 a9-b0 38 bb 7c d0 ae be bf   b........8.|....
    //    000007D0  74 47 16 a0 a2 d3 fc 1d-b9 ba 26 10 06 ef ba 1d   tG........&.....
    //    000007E0  43 01 4e 4e 6f 56 ca e0-ee d0 f9 4e a6 62 63 ff   C.NNoV.....N.bc.
    //    000007F0  da 0b c9 15 61 6c ed 6b-0b c4 58 53 86 0f 8c 0c   ....al.k..XS....
    //    00000800  1a 2e df c1 f2 43 48 d4-af 0a 78 36 b2 51 32 28   .....CH...x6.Q2(
    //    00000810  6c c2 75 79 3f 6e 99 66-88 3e 34 d3 7f 6d 9d 07   l.uy?n.f.>4..m..
    //    00000820  e4 6b eb 84 e2 0a bb ca-7d 3a 40 71 b0 be 47 9f   .k......}:@q..G.
    //    00000830  12 58 31 61 2b 9b 4a 9a-49 8f e5 b4 0c f5 04 4d   .X1a+.J.I......M
    //    00000840  3c ce bc d2 79 15 d9 28-f4 23 56 77 9f 38 64 3e   <...y..(.#Vw.8d>
    //    00000850  03 88 92 04 26 76 b9 b5-df 19 d0 78 4b 7a 60 40   ....&v.....xKz`@
    //    00000860  23 91 f1 15 22 2b b4 e7-02 54 a9 16 21 5b 60 96   #..."+...T..![`.
    //    00000870  a9 5c 00 00 00 00 00 00-00 00 00 00 00 00 00 00   .\..............
    //    00000880  00 00 01 00 00 00 0e 00-0e 00 6d 69 63 72 6f 73   ..........micros
    //    00000890  6f 66 74 2e 63 6f 6d 00                           oft.com.

    //    0x00: LICENSE_PREAMBLE (4 Bytes)
    //    01 -> LICENSE_PREAMBLE::bMsgType = LICENSE_REQUEST (1 Byte)

    //    03 -> LICENSE_PREAMBLE::bVersion = 3 (RDP 5.0, 5.2, 6.0) (1 Byte)

    //    98 -\|
    //    08 -/ LICENSE_PREAMBLE::wMsgSize = 0x898 (2 Bytes)

    //    0x04: ServerRandom (0x20 Bytes)
    //    84 ef ae 20 b1 d5 9e 36 -\|
    //    49 1a e8 2e 0a 99 89 ac -|
    //    49 a6 47 4f-33 9b 5a b9 -|
    //    95 03 a6 c6 c2 3c 3f 61 -/ SERVER_RANDOM

    //    0x24: Product Info (4 + 4 + 0x2c + 4 + 8 = 0x40 bytes)
    //    00 -\|
    //    00 -|
    //    06 -|
    //    00 -/ PRODUCT_INFO::dwVersion = 0x00060000

    //    2c -\|
    //    00 -|
    //    00 -|
    //    00 -/ PRODUCT_INFO::cbCompanyName = 0x2c (44 bytes)

    //    4d 00 69 00 63 00 72 00 -\|
    //    6f 00 73 00 6f 00 66 00 -|
    //    74 00 20 00 43 00 6f 00 -|
    //    72 00 70 00 6f 00 72 00 -|
    //    61 00 74 00 69 00 6f 00 -|
    //    6e 00 00 00             -/ PRODUCT_INFO::pbCompanyName

    //    08 -\|
    //    00 -|
    //    00 -|
    //    00 -/ PRODUCT_INFO::cbProductId = 0x8 (8 bytes)

    //    41 00 30 00 32 00 00 00 -> PRODUCT_INFO::pbProductId
    //    0x64: KeyExchangeList (2 + 2 + 4 = 8 bytes)
    //    0d -\|
    //    00 -/ KeyExchangeList::wBlobType = BB_KEY_EXCHG_ALG_BLOB

    //    04 -\|
    //    00 -/ KeyExchangeList::wBlobLen = 4

    //    01 -\|
    //    00 -|
    //    00 -|
    //    00 -/ KeyExchangeList::pBlob

    //    0x6c: Server Certificate (2 + 2 + 0x812 = 0x816 bytes)
    //    03 -\|
    //    00 -/ ServerCertificate::wBlobType = BB_CERTIFICATE_BLOB
    //     
    //    12 -\|
    //    08 -/ ServerCertificate::wBlobLen = 0x812 bytes
    //     

    //                02 00 00 80-02 00 00 00 f5 02 00 00 -\|
    //    30 82 02 f1 30 82 01 dd-a0 03 02 01 02 02 08 01 -|
    //    9e 24 a2 f2 ae 90 80 30-09 06 05 2b 0e 03 02 1d -|
    //    05 00 30 32 31 30 30 13-06 03 55 04 03 1e 0c 00 -|
    //    52 00 4f 00 44 00 45 00-4e 00 54 30 19 06 03 55 -|
    //    04 07 1e 12 00 57 00 4f-00 52 00 4b 00 47 00 52 -|
    //    00 4f 00 55 00 50 30 1e-17 0d 37 30 30 35 32 37 -|
    //    30 31 31 31 30 33 5a 17-0d 34 39 30 35 32 37 30 -|
    //    31 31 31 30 33 5a 30 32-31 30 30 13 06 03 55 04 -|
    //    03 1e 0c 00 52 00 4f 00-44 00 45 00 4e 00 54 30 -|
    //    19 06 03 55 04 07 1e 12-00 57 00 4f 00 52 00 4b -|
    //    00 47 00 52 00 4f 00 55-00 50 30 82 01 22 30 0d -|
    //    06 09 2a 86 48 86 f7 0d-01 01 01 05 00 03 82 01 -|
    //    0f 00 30 82 01 0a 02 82-01 01 00 88 ad 7c 8f 8b -|
    //    82 76 5a bd 8f 6f 62 18-e1 d9 aa 41 fd ed 68 01 -|
    //    c6 34 35 b0 29 04 ca 4a-4a 1c 7e 80 14 f7 8e 77 -|
    //    b8 25 ff 16 47 6f bd e2-34 3d 2e 02 b9 53 e4 33 -|
    //    75 ad 73 28 80 a0 4d fc-6c c0 22 53 1b 2c f8 f5 -|
    //    01 60 19 7e 79 19 39 8d-b5 ce 39 58 dd 55 24 3b -|
    //    55 7b 43 c1 7f 14 2f b0-64 3a 54 95 2b 88 49 0c -|
    //    61 2d ac f8 45 f5 da 88-18 5f ae 42 f8 75 c7 26 -|
    //    6d b5 bb 39 6f cc 55 1b-32 11 38 8d e4 e9 44 84 -|
    //    11 36 a2 61 76 aa 4c b4-e3 55 0f e4 77 8e de e3 -|
    //    a9 ea b7 41 94 00 58 aa-c9 34 a2 98 c6 01 1a 76 -|
    //    14 01 a8 dc 30 7c 77 5a-20 71 5a a2 3f af 13 7e -|
    //    e8 fd 84 a2 5b cf 25 e9-c7 8f a8 f2 8b 84 c7 04 -|
    //    5e 53 73 4e 0e 89 a3 3c-e7 68 5c 24 b7 80 53 3c -|
    //    54 c8 c1 53 aa 71 71 3d-36 15 d6 6a 9d 7d de ae -|
    //    f9 e6 af 57 ae b9 01 96-5d e0 4d cd ed c8 d7 f3 -|
    //    01 03 38 10 be 7c 42 67-01 a7 23 02 03 01 00 01 -|
    //    a3 13 30 11 30 0f 06 03-55 1d 13 04 08 30 06 01 -|
    //    01 ff 02 01 00 30 09 06-05 2b 0e 03 02 1d 05 00 -|
    //    03 82 01 01 00 81 dd d2-d3 33 d4 a3 b6 8e 6e 7d -|
    //    9f fd 73 9f 31 0b dd 42-82 3f 7e 21 df 28 cc 59 -|
    //    ca 6a c0 a9 3d 30 7d e1-91 db 77 6b 8b 10 e6 fd -|
    //    bc 3c a3 58 48 c2 36 dd-a0 0b f5 8e 13 da 7b 04 -|
    //    08 44 b4 f2 a8 0d 1e 0b-1d 1a 3f f9 9b 4b 5a 54 -|
    //    c5 b3 b4 03 93 75 b3 72-5c 3d cf 63 0f 15 e1 64 -|
    //    58 de 52 8d 97 79 0e a4-34 d5 66 05 58 b8 6e 79 -|
    //    b2 09 86 d5 f0 ed c4 6b-4c ab 02 b8 16 5f 3b ed -|
    //    88 5f d1 de 44 e3 73 47-21 f7 03 ce e1 6d 10 0f -|
    //    95 cf 7c a2 7a a6 bf 20-db e1 93 04 c8 5e 6a be -|
    //    c8 01 5d 27 b2 03 0f 66-75 e7 cb ea 8d 4e 98 9d -|
    //    22 ed 28 40 d2 7d a4 4b-ef cc bf 01 2a 6d 3a 3e -|
    //    be 47 38 f8 ea a4 c6 30-1d 5e 25 cf fb e8 3d 42 -|
    //    dd 29 e8 99 89 9e bf 39-ee 77 09 d9 3e 8b 52 36 -|
    //    b6 bb 8b bd 0d b2 52 aa-2c cf 38 4e 4d cf 1d 6d -|
    //    5d 25 17 ac 2c f6 f0 65-5a c9 fe 31 53 b4 f0 0c -|
    //    94 4e 0d 54 8e fd 04 00-00 30 82 04 f9 30 82 03 -|
    //    e5 a0 03 02 01 02 02 05-01 00 00 00 02 30 09 06 -|
    //    05 2b 0e 03 02 1d 05 00-30 32 31 30 30 13 06 03 -|
    //    55 04 03 1e 0c 00 52 00-4f 00 44 00 45 00 4e 00 -|
    //    54 30 19 06 03 55 04 07-1e 12 00 57 00 4f 00 52 -|
    //    00 4b 00 47 00 52 00 4f-00 55 00 50 30 1e 17 0d -|
    //    30 37 30 35 32 36 31 32-34 35 35 33 5a 17 0d 33 -|
    //    38 30 31 31 39 30 33 31-34 30 37 5a 30 81 92 31 -|
    //    81 8f 30 23 06 03 55 04-03 1e 1c 00 6e 00 63 00 -|
    //    61 00 6c 00 72 00 70 00-63 00 3a 00 52 00 4f 00 -|
    //    44 00 45 00 4e 00 54 30-23 06 03 55 04 07 1e 1c -|
    //    00 6e 00 63 00 61 00 6c-00 72 00 70 00 63 00 3a -|
    //    00 52 00 4f 00 44 00 45-00 4e 00 54 30 43 06 03 -|
    //    55 04 05 1e 3c 00 31 00-42 00 63 00 4b 00 65 00 -|
    //    62 00 68 00 70 00 58 00-5a 00 74 00 4c 00 71 00 -|
    //    4f 00 37 00 53 00 51 00-6e 00 42 00 70 00 52 00 -|
    //    66 00 75 00 64 00 64 00-64 00 59 00 3d 00 0d 00 -|
    //    0a 30 82 01 1e 30 09 06-05 2b 0e 03 02 0f 05 00 -|
    //    03 82 01 0f 00 30 82 01-0a 02 82 01 01 00 c8 90 -|
    //    6b f0 c6 58 81 a6 89 1c-0e f2 f6 d9 82 12 71 a5 -|
    //    6e 51 db e0 32 66 aa 91-77 0e 88 ab 44 b7 d3 97 -|
    //    da 78 8f 0e 44 26 46 7f-16 d4 c6 63 eb ca 55 e5 -|
    //    4e 8b 2d a6 6d 83 95 a7-a8 6a fa d0 be 26 80 ae -|
    //    ab 0a 64 90 32 8c df 5c-f8 f9 d0 7e d1 6b 3a 29 -|
    //    7e 7d bd 02 a3 86 6c fd-a5 35 71 da 21 b4 ee a4 -|
    //    97 f3 a8 b2 12 db a4 27-57 36 c9 08 22 5c 54 f7 -|
    //    99 7b a3 2f b8 5c d5 16-b8 19 27 6b 71 97 14 5b -|
    //    e8 1f 23 e8 5c b8 1b 73-4b 6e 7a 03 13 ff 97 e9 -|
    //    62 b9 4a a0 51 23 c3 6c-32 3e 02 f2 63 97 23 1c -|
    //    c5 78 d8 fc b7 07 4b b0-56 0f 74 df c5 56 28 e4 -|
    //    96 fd 20 8e 65 5a e6 45-ed c1 05 3e ab 58 55 40 -|
    //    af e2 47 a0 4c 49 a3 8d-39 e3 66 5f 93 33 6d f8 -|
    //    5f c5 54 e5 fb 57 3a de-45 12 b5 c7 05 4b 88 1f -|
    //    b4 35 0f 7c c0 75 17 c6-67 dd 48 80 cb 0a be 9d -|
    //    f6 93 60 65 34 eb 97 af-65 6d df bf 6f 5b 02 03 -|
    //    01 00 01 a3 82 01 bf 30-82 01 bb 30 14 06 09 2b -|
    //    06 01 04 01 82 37 12 04-01 01 ff 04 04 01 00 05 -|
    //    00 30 3c 06 09 2b 06 01-04 01 82 37 12 02 01 01 -|
    //    ff 04 2c 4d 00 69 00 63-00 72 00 6f 00 73 00 6f -|
    //    00 66 00 74 00 20 00 43-00 6f 00 72 00 70 00 6f -|
    //    00 72 00 61 00 74 00 69-00 6f 00 6e 00 00 00 30 -|
    //    81 cd 06 09 2b 06 01 04-01 82 37 12 05 01 01 ff -|
    //    04 81 bc 00 30 00 00 01-00 00 00 02 00 00 00 09 -|
    //    04 00 00 1c 00 4a 00 66-00 4a 00 b0 00 01 00 33 -|
    //    00 64 00 32 00 36 00 37-00 39 00 35 00 34 00 2d -|
    //    00 65 00 65 00 62 00 37-00 2d 00 31 00 31 00 64 -|
    //    00 31 00 2d 00 62 00 39-00 34 00 65 00 2d 00 30 -|
    //    00 30 00 63 00 30 00 34-00 66 00 61 00 33 00 30 -|
    //    00 38 00 30 00 64 00 00-00 33 00 64 00 32 00 36 -|
    //    00 37 00 39 00 35 00 34-00 2d 00 65 00 65 00 62 -|
    //    00 37 00 2d 00 31 00 31-00 64 00 31 00 2d 00 62 -|
    //    00 39 00 34 00 65 00 2d-00 30 00 30 00 63 00 30 -|
    //    00 34 00 66 00 61 00 33-00 30 00 38 00 30 00 64 -|
    //    00 00 00 00 00 00 10 00-80 64 00 00 00 00 00 30 -|
    //    6e 06 09 2b 06 01 04 01-82 37 12 06 01 01 ff 04 -|
    //    5e 00 30 00 00 00 00 0e-00 3e 00 52 00 4f 00 44 -|
    //    00 45 00 4e 00 54 00 00-00 37 00 38 00 34 00 34 -|
    //    00 30 00 2d 00 30 00 30-00 36 00 2d 00 35 00 38 -|
    //    00 36 00 37 00 30 00 34-00 35 00 2d 00 37 00 30 -|
    //    00 33 00 34 00 37 00 00-00 57 00 4f 00 52 00 4b -|
    //    00 47 00 52 00 4f 00 55-00 50 00 00 00 00 00 30 -|
    //    25 06 03 55 1d 23 01 01-ff 04 1b 30 19 a1 10 a4 -|
    //    0e 52 00 4f 00 44 00 45-00 4e 00 54 00 00 00 82 -|
    //    05 01 00 00 00 02 30 09-06 05 2b 0e 03 02 1d 05 -|
    //    00 03 82 01 01 00 2e eb-c7 0d b8 1d 47 11 9d 09 -|
    //    88 9b 51 dc 45 dd 56 51-e2 d1 23 11 39 9b 2d da -|
    //    c7 fe 7a d7 84 e3 3d 54-77 97 4d 19 92 30 64 a0 -|
    //    47 c6 2f 6d 93 d2 64 7c-76 c8 26 45 ad 5a 44 54 -|
    //    ea f6 4b 28 77 1f 77 ea-ec 74 02 38 68 9e 79 14 -|
    //    72 83 34 74 62 d2 c1 0c-a4 0b f2 a9 b0 38 bb 7c -|
    //    d0 ae be bf 74 47 16 a0-a2 d3 fc 1d b9 ba 26 10 -|
    //    06 ef ba 1d 43 01 4e 4e-6f 56 ca e0 ee d0 f9 4e -|
    //    a6 62 63 ff da 0b c9 15-61 6c ed 6b 0b c4 58 53 -|
    //    86 0f 8c 0c 1a 2e df c1-f2 43 48 d4 af 0a 78 36 -|
    //    b2 51 32 28 6c c2 75 79-3f 6e 99 66 88 3e 34 d3 -|
    //    7f 6d 9d 07 e4 6b eb 84-e2 0a bb ca 7d 3a 40 71 -|
    //    b0 be 47 9f 12 58 31 61-2b 9b 4a 9a 49 8f e5 b4 -|
    //    0c f5 04 4d 3c ce bc d2-79 15 d9 28 f4 23 56 77 -|
    //    9f 38 64 3e 03 88 92 04-26 76 b9 b5 df 19 d0 78 -|
    //    4b 7a 60 40 23 91 f1 15-22 2b b4 e7 02 54 a9 16 -|
    //    21 5b 60 96 a9 5c 00 00-00 00 00 00 00 00 00 00 -|
    //    00 00 00 00 00 00                               -/
    //    ServerCertificate::pBlob


    //    0x882: ScopeList
    //    01 -\|
    //    00 -|
    //    00 -|
    //    00 -/ ScopeList::ScopeCount

    //    0e 00 0e 00 6d 69 63 72 -\|
    //    6f 73 6f 66 74 2e 63 6f -|
    //    6d 00                   -/ ScopeList::ScopeArray


    struct LicenseRequest_Recv
    {
        uint8_t tag;
        uint8_t flags;
        uint16_t wMsgSize;
        uint8_t server_random[SEC_RANDOM_SIZE];

        LicenseRequest_Recv(Stream & stream){
            this->tag = stream.in_uint8();
            this->flags = stream.in_uint8();
            this->wMsgSize = stream.in_uint16_le();
         
            stream.in_copy_bytes(this->server_random, SEC_RANDOM_SIZE);

            TODO("Add missing productInfo field")
            stream.end = stream.p;

            if (stream.p != stream.end){
                LOG(LOG_ERR, "License Request_Recv : unparsed data %d", stream.end - stream.p);
                throw Error(ERR_LIC);
            }
        }
    };

    //    3.2.1.6 Platform Challenge
    //    --------------------------
    //    The platform challenge is a random string generated by the server. This string is encrypted (see
    //    Encrypting Licensing Data (section 5.1.4)) with the licensing encryption key using RC4 and sent in
    //    the EncryptedPlatformChallenge field of the Server Platform Challenge message. It is created at
    //    the beginning of the licensing protocol and destroyed when the licensing protocol is completed.


    //    4.4 SERVER PLATFORM CHALLENGE
    //    -----------------------------

    //    The server sends the platform challenge to the client to authenticate the client. 
    //    See sections 2.2.2.4 and 3.2.5.4.

    //    00000000:  02 03 26 00 ff ff ff ff-50 f7 0a 00 46 37 85 54  ..&.....P...F7.T
    //    00000010:  8e c5 91 34 97 5d 78 94-ad 3b 81 da 88 18 56 0f  ...4.]x..;....V.
    //    00000020:  3a d1 f1 03 ef 35                                :....5

    //    0x00: LICENSE_PREAMBLE (4 bytes)
    //    02 -> LICENSE_PREAMBLE::bMsgType = SERVER_PLATFORM_CHALLENGE
    //     
    //    03 -> LICENSE_PREAMBLE::bVersion = 3
    //     
    //    26 -\|
    //    00 -/ LICENSE_PREAMBLE::wMsgSize
    //    0x04: ConnectFlags (4 bytes)
    //    ff -\|
    //    ff -|
    //    ff -|
    //    ff -/ dwConnectFlags (ignored)
    //    0x08: EncryptedPlatformChallenge (2 + 2 + 0xa = 0xe bytes)
    //    50 -\|
    //    f7 -/ EncryptedPlatformChallenge::wBlobType (ignored)
    //     
    //    0a -\|
    //    00 -/ EncryptedPlatformChallenge::wBlobLen
    //     
    //    46 37 85 54 8e c5 91 34 -\|
    //                      97 5d -/ EncryptedPlatformChallenge::pBlob

    //    The corresponding decrypted blob for the above is
    //    0x00000000  54 00 45 00 53 00 54 00 00 00  T.E.S.T...

    //    0x16: MACData
    //    78 94 ad 3b 81 da 88 18 -\|
    //    56 0f 3a d1 f1 03 ef 35 -/ MACData

    struct PlatformChallenge_Recv
    {
        uint8_t wMsgType;
        uint8_t bVersion;
        uint16_t wMsgSize;
        uint32_t dwConnectFlags;
        struct EncryptedPlatformChallenge {
            uint16_t wBlobType;
            uint16_t wBlobLen;
            uint8_t blob[LICENSE_TOKEN_SIZE];
        } encryptedPlatformChallenge;
        uint8_t MACData[LICENSE_SIGNATURE_SIZE];

        PlatformChallenge_Recv(Stream & stream){
            this->wMsgType = stream.in_uint8();
            this->bVersion = stream.in_uint8();
            this->wMsgSize = stream.in_uint16_le();
            this->dwConnectFlags = stream.in_uint32_le(); // ignored
            this->encryptedPlatformChallenge.wBlobType = stream.in_uint16_le();      // ignored
            this->encryptedPlatformChallenge.wBlobLen = stream.in_uint16_le();
            if (this->encryptedPlatformChallenge.wBlobLen != LICENSE_TOKEN_SIZE) {
                LOG(LOG_ERR, "token len = %d, expected %d", this->encryptedPlatformChallenge.wBlobLen, LICENSE_TOKEN_SIZE);
                throw Error(ERR_LIC);
            }
            stream.in_copy_bytes(this->encryptedPlatformChallenge.blob, LICENSE_TOKEN_SIZE);
            stream.in_copy_bytes(this->MACData, LICENSE_SIGNATURE_SIZE);

            if (stream.p != stream.end){
                LOG(LOG_ERR, "PlatformChallenge_Recv : unparsed data %d", stream.end - stream.p);
                throw Error(ERR_LIC);
            }
        }
    };

};


// 2.2.1.12 Server License Error PDU - Valid Client
// =============================================

// The License Error (Valid Client) PDU is an RDP Connection Sequence PDU sent
// from server to client during the Licensing phase of the RDP Connection
// Sequence (see section 1.3.1.1 for an overview of the RDP Connection Sequence
// phases). This licensing PDU indicates that the server will not issue the
// client a license to store and that the Licensing Phase has ended successfully.

// This is one possible licensing PDU that may be sent during the Licensing
// Phase (see [MS-RDPELE] section 2.2.2 for a list of all permissible licensing
// PDUs).

// tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

// x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in [X224] section 13.7.

// mcsSDin (variable): Variable-length PER-encoded MCS Domain PDU (DomainMCSPDU) which encapsulates an MCS Send Data Indication structure (SDin, choice 26 from DomainMCSPDU), as specified in [T125] section 11.33 (the ASN.1 structure definitions are given in [T125] section 7, parts 7 and 10). The userData field of the MCS Send Data Indication contains a Security Header and a Valid Client License Data (section 2.2.1.12.1) structure.

// securityHeader (variable): Security header. The format of the security header depends on the Encryption Level and Encryption Method selected by the server (sections 5.3.2 and 2.2.1.4.3).

// This field MUST contain one of the following headers:

// - Basic Security Header (section 2.2.8.1.1.2.1) if the Encryption Level selected by the server is ENCRYPTION_LEVEL_NONE (0) or ENCRYPTION_LEVEL_LOW (1) and the embedded flags field does not contain the SEC_ENCRYPT (0x0008) flag.

// - Non-FIPS Security Header (section 2.2.8.1.1.2.2) if the Encryption Method selected by the server is ENCRYPTION_METHOD_40BIT (0x00000001), ENCRYPTION_METHOD_56BIT (0x00000008), or ENCRYPTION_METHOD_128BIT (0x00000002) and the embedded flags field contains the SEC_ENCRYPT (0x0008) flag.

// - FIPS Security Header (section 2.2.8.1.1.2.3) if the Encryption Method selected by the server is ENCRYPTION_METHOD_FIPS (0x00000010) and the embedded flags field contains the SEC_ENCRYPT (0x0008) flag.

// If the Encryption Level is set to ENCRYPTION_LEVEL_CLIENT_COMPATIBLE (2), ENCRYPTION_LEVEL_HIGH (3), or ENCRYPTION_LEVEL_FIPS (4) and the flags field of the security header does not contain the SEC_ENCRYPT (0x0008) flag (the licensing PDU is not encrypted), then the field MUST contain a Basic Security Header. This MUST be the case if SEC_LICENSE_ENCRYPT_SC (0x0200) flag was not set on the Security Exchange PDU (section 2.2.1.10).

// The flags field of the security header MUST contain the SEC_LICENSE_PKT (0x0080) flag (see Basic (TS_SECURITY_HEADER)).

// validClientLicenseData (variable): The actual contents of the License Error (Valid Client) PDU, as specified in section 2.2.1.12.1.

// 2.2.1.12.1 Valid Client License Data (LICENSE_VALID_CLIENT_DATA)
// The LICENSE_VALID_CLIENT_DATA structure contains information which indicates that the server will not issue the client a license to store and that the Licensing Phase has ended successfully.

// preamble (4 bytes): Licensing Preamble (section 2.2.1.12.1.1) structure containing header information. The bMsgType field of the preamble structure MUST be set to ERROR_ALERT (0xFF).

// validClientMessage (variable): A Licensing Error Message (section 2.2.1.12.1.3) structure.

// The dwErrorCode field of the error message structure MUST be set to STATUS_VALID_CLIENT (0x00000007) and the dwStateTransition field MUST be set to ST_NO_TRANSITION (0x00000002). The bbErrorInfo field MUST contain an empty binary large object (BLOB) of type BB_ERROR_BLOB (0x0004).

// 2.2.1.12.1.1 Licensing Preamble (LICENSE_PREAMBLE)
// ==================================================
// The LICENSE_PREAMBLE structure precedes every licensing packet sent on the wire.

// bMsgType (1 byte): An 8-bit, unsigned integer. A type of the licensing
// packet. For more details about the different licensing packets, see
// [MS-RDPELE] section 2.2.2.

// Sent by server:
// ---------------
// 0x01 LICENSE_REQUEST Indicates a License Request PDU ([MS-RDPELE] section 2.2.2.1).
// 0x02 PLATFORM_CHALLENGE Indicates a Platform Challenge PDU ([MS-RDPELE] section 2.2.2.4).
// 0x03 NEW_LICENSE Indicates a New License PDU ([MS-RDPELE] section 2.2.2.7).
// 0x04 UPGRADE_LICENSE Indicates an Upgrade License PDU ([MS-RDPELE] section 2.2.2.6).
//
// Sent by client:
// ---------------
// 0x12 LICENSE_INFO Indicates a License Information PDU ([MS-RDPELE] section 2.2.2.3).
// 0x13 NEW_LICENSE_REQUEST Indicates a New License Request PDU ([MS-RDPELE] section 2.2.2.2).
// 0x15 PLATFORM_CHALLENGE_RESPONSE Indicates a Platform Challenge Response PDU ([MS-RDPELE] section 2.2.2.5).

// Sent by either client or server:
// --------------------------------
// 0xFF ERROR_ALERT Indicates a Licensing Error Message PDU (section 2.2.1.12.1.3).

// flags (1 byte): An 8-bit unsigned integer. License preamble flags.
// 0x0F LicenseProtocolVersionMask The license protocol version. See the discussion which follows this table for more information.
// 0x80 EXTENDED_ERROR_MSG_SUPPORTED Indicates that extended error information using the License Error Message (section 2.2.1.12.1.3) is supported.

// The LicenseProtocolVersionMask is a 4-bit value containing the supported license protocol version. The following are possible version values.

// wMsgSize (2 bytes): An 16-bit, unsigned integer. The size in bytes of the licensing packet (including the size of the preamble).

// 2.2.2.1 Server License Request (SERVER_LICENSE_REQUEST)
// =======================================================

// See MS-RDPELE for details


    // 2.2.2.5 Client Platform Challenge Response (CLIENT_PLATFORM_CHALLENGE_RESPONSE)
    // ===============================================================================

    // The Client Platform Challenge Response packet is sent by the client in response
    // to the Server Platform Challenge (section 2.2.2.4) message.

    // EncryptedPlatformChallengeResponse (variable): A LICENSE_BINARY_BLOB<14>
    // structure (as specified in [MS-RDPBCGR] section 2.2.1.12.1.2) of wBlobType
    // BB_ENCRYPTED_DATA_BLOB (0x0009). This BLOB contains the encrypted Platform
    // Challenge Response Data (section 2.2.2.5.1) generated by the client and is
    // encrypted with the licensing encryption key (see section 5.1.3), using RC4
    // (for instructions on how to perform the encryption, see section 5.1.4).

    // 2.2.2.5.1 Platform Challenge Response Data (PLATFORM_CHALLENGE_RESPONSE_DATA)
    // ------------------------------------------------------------------------------

    // The Platform Challenge Response Data packet contains information pertaining
    // to the client's license handling capabilities and the Client Platform Challenge
    // data sent by the server in the Server Platform Challenge.

    // wVersion (2 bytes): A 16-bit unsigned integer that contains the platform
    // challenge version. This field MUST be set to 0x0100.

    // wClientType (2 bytes): A 16-bit unsigned integer that represents the
    // operating system type of the client and MAY contain one of following values.<15>

    // +-------------------------------------+--------------------------------+
    // | 0x0100 WIN32_PLATFORMCHALLENGE_TYPE | Win32 Platform Challenge Type. |
    // +-------------------------------------+--------------------------------+
    // | 0x0200 WIN16_PLATFORMCHALLENGE_TYPE | Win16 Platform Challenge Type. |
    // +-------------------------------------+--------------------------------+
    // | 0x0300 WINCE_PLATFORMCHALLENGE_TYPE | WinCE Platform Challenge Type. |
    // +-------------------------------------+--------------------------------+
    // | 0xFF00 OTHER_PLATFORMCHALLENGE_TYPE | Other Platform Challenge Type. |
    // +-------------------------------------+--------------------------------+

    // wLicenseDetailLevel (2 bytes): A 16-bit unsigned integer. This field
    // represents the capability of the client to handle license data. RDP
    // version 5.0 and later clients SHOULD advertise support for large (6.5 KB
    // or higher) licenses by setting the detail level to LICENSE_DETAIL_DETAIL
    // (0x0003). The following table lists valid values for this field.

    // +--------------------------------+---------------------------------------+
    // | 0x0001 LICENSE_DETAIL_SIMPLE   | License Detail Simple (client license |
    // |                                | certificate and license server        |
    // |                                | certificate without issuer details).  |
    // +--------------------------------+---------------------------------------+
    // | 0x0002 LICENSE_DETAIL_MODERATE | License Detail Moderate (client       |
    // |                                | license certificate chain up to       |
    // |                                | license server's certificate issuer). |
    // +--------------------------------+---------------------------------------+
    // | 0x0003 LICENSE_DETAIL_DETAIL   | License Detail Detail (client license |
    // |                                | certificate chain up to root          |
    // |                                | certificate).                         |
    // +--------------------------------+---------------------------------------+

    // cbChallenge (2 bytes): A 16-bit unsigned integer that indicates the number
    // of bytes of binary data contained in the pbChallenge field.

    // pbChallenge (variable): Contains the decrypted Client Platform Challenge
    // data sent by the server in the Server Platform Challenge message.

    // ---------------- End of Platform Challenge Data ------------------------

    // EncryptedHWID (variable): A LICENSE_BINARY_BLOB structure (as specified in
    // [MS-RDPBCGR] section 2.2.1.12.1.2) of wBlobType BB_ENCRYPTED_DATA_BLOB (0x0009).
    // This BLOB contains the encrypted Client Hardware Identification (section 2.2.2.3.1)
    // and is encrypted with the licensing encryption key (see section 5.1.3) using RC4
    // (for instructions on how to perform the encryption, see section 5.1.4).

    // MACData (16 bytes): An array of 16 bytes containing an MD5 digest (MAC)
    // generated over the decrypted Client Hardware Identification and Platform
    // Challenge Response Data. For instructions on how to generate this message
    // digest, see section 5.1.6; for a description of how the server uses the
    // MACData field to verify the integrity of the Client Hardware Identification
    // and the Platform Challenge Response Data, see section 3.1.5.1.


    // 2.2.2.2 Client New License Request (CLIENT_NEW_LICENSE_REQUEST)
    // ===============================================================
    // The Client New License Request packet is sent to a server when the client
    // cannot find a license matching the product information provided in the
    // Server License Request message. This message is interpreted as a new
    // license request by the server, and the server SHOULD attempt to issue
    // a new license to the client on receipt of this message.

    // PreferredKeyExchangeAlg (4 bytes): A 32-bit unsigned integer that
    // indicates the key exchange algorithm chosen by the client. It MUST be set
    // to KEY_EXCHANGE_ALG_RSA (0x00000001), which indicates an RSA-based key
    // exchange with a 512-bit asymmetric key.<9>

    // PlatformId (4 bytes): A 32-bit unsigned integer. This field is composed
    // of two identifiers: the operating system identifier and the independent
    // software vendor (ISV) identifier. The platform ID is composed of the
    // logical OR of these two values.

    // The most significant byte of the PlatformId field contains the operating
    // system version of the client.<10>

    // The second most significant byte of the PlatformId field identifies the
    // ISV that provided the client image.<11>

    // The remaining two bytes in the PlatformId field are used by the ISV to
    // identify the build number of the operating system.<12>

    // ClientRandom (32 bytes): A 32-byte random number generated by the client
    // using a cryptographically secure pseudo-random number generator. The
    // ClientRandom and ServerRandom (see section 2.2.2.1) values, along with
    // the data in the EncryptedPreMasterSecret field, are used to generate
    // licensing encryption keys (see section 5.1.3). These keys are used to
    // encrypt licensing protocol messages (see sections 5.1.4 and 5.1.5).

    // EncryptedPreMasterSecret (variable): A Licensing Binary BLOB structure
    // (see [MS-RDPBCGR] section 2.2.1.12.1.2) of type BB_RANDOM_BLOB (0x0002).
    // This BLOB contains an encrypted 48-byte random number. For instructions
    // on how to encrypt this random number, see section 5.1.2.1.

    // ClientUserName (variable): A Licensing Binary BLOB structure (see
    // [MS-RDPBCGR] section 2.2.1.12.1.2) of type BB_CLIENT_USER_NAME_BLOB
    // (0x000F). This BLOB contains the client user name string in
    // null-terminated ANSI character set format and is used along with the
    // ClientMachineName BLOB to keep track of licenses issued to clients.

    // ClientMachineName (variable): A Licensing Binary BLOB structure (see
    // [MS-RDPBCGR] section 2.2.1.12.1.2) of type BB_CLIENT_MACHINE_NAME_BLOB
    // (0x0010). This BLOB contains the client machine name string in
    // null-terminated ANSI character set format and is used along with the
    // ClientUserName BLOB to keep track of licenses issued to clients.


// GLOSSARY
// ========

// clearing house: A Microsoft central authority for activating a license server and registering client access licenses (CALs).

// client access license (CAL): A license required by a client user or device for accessing a terminal server configured in Application Server mode.

// client license: See client access license (CAL).

// grace period: The duration of time during which a terminal server allows clients to connect without requiring a CAL. The grace period ends either when the duration is complete or when the terminal server receives the first permanent license from the license server.

// license encryption key: A shared symmetric key generated by both the server and client that is used to encrypt licensing message data.

// license server: A server that issues CALs.

// license server certificate: An X.509 certificate used for signing CALs.

// license store: A client-side database that stores CALs issued by a terminal server.

// MD5 digest: A 128-bit message hash value generated as output by the MD5 Message-Digest algorithm. See [RFC1321].

// Message Authentication Code (MAC): A generated value used to verify the integrity of a received licensing message.

// object identifier (OID): A number that uniquely identifies an object class or attribute in a system. See [MSDN-OBJID].

// permanent license: A CAL issued to authenticated clients.

// personal terminal server: In general context, refers to a client SKU target machine that hosts remote desktop sessions. From a terminal service licensing perspective, the behavior of a personal terminal server is similar to that of a terminal server in remote administration mode. Thus any behavioral reference to a personal terminal server in this document essentially implies that the particular behavior is valid for a terminal server in remote administration mode as well. The term personal terminal server is therefore used to encompass all connections where either the end point is a client SKU operating system or is a terminal server running in remote administration mode.

// premaster secret: A 48-byte random number used in license encryption key generation.

// remote administration mode: A terminal server may function in remote administration mode if either the terminal services role is not installed on the machine or the client used to invoke the session has enabled the /admin switch.<1>

// Remote Desktop client: A device that connects to a terminal server and renders the user interface through which a user interacts with a remote session.

// session encryption key: A shared key used for confidential exchange of data between the client and the server.

// temporary license: A type of CAL issued by a terminal server to a client in situations in which a permanent license is not available.

// terminal server: A server that hosts Remote Desktop sessions and enables interaction with each of these sessions on a connected client device. A reference to terminal server in this document generally implies a terminal server in app-server mode.

// terminal server certificate: A certificate that should be used to authenticate a terminal server.

// MAY, SHOULD, MUST, SHOULD NOT, MUST NOT: These terms (in all caps) are used as specified in [RFC2119]. All statements of optional behavior use either MAY, SHOULD, or SHOULD NOT.

// 3.2.5.5 Processing Client Platform Challenge Responses
// ===================================================

// When a server receives the Client Platform Challenge Response message, it decrypts the EncryptedPlatformChallengeResponse and EncryptedHWID fields in the message using the license encryption key generated while processing earlier licensing messages.

// The server MUST then generate the MAC checksum over the decrypted Platform Challenge Response Data packet and decrypted Client Hardware Identification packet, and MUST compare it with the received MAC checksum to verify the data integrity. Handling invalid MACs (section 3.2.5.9) is specified in section 3.2.5.9.

// The following cases can result:

// Case 1: A Client License Information message was received earlier by the server, and the CAL (LicenseInfo BLOB) in the message required an upgrade.

// If the license server cannot be contacted to upgrade the license, and the old license is still valid, the terminal server sends the Server Upgrade License message and returns the old license to the client.

// Case 2: If either of the following conditions occurs:

//    The CAL (LicenseInfo BLOB) received in the Client License Information message required an upgrade, and the license server cannot be contacted to upgrade the CAL and the old license is not valid.

// Or:

//    A Client New License Request message was received earlier, and the license server cannot be contacted to issue a new CAL.

// In this case, if the server's grace period has not been exceeded, the server responds as if the client presented a valid license by sending a license error message with an error code of STATUS_VALID_CLIENT (0x00000007) and a state transition code of ST_NO_TRANSITION (0x00000002), ending the licensing protocol.

// If the server's grace period has been exceeded, it sends a license error message with error code ERR_NO_LICENSE_SERVER (0x00000006) and a state transition of ST_TOTAL_ABORT (0x00000001). The licensing protocol is aborted.

// Case 3: If either of the following conditions occurs:

//    The CAL (LicenseInfo BLOB) received in the Client License Information message required an upgrade, and the license server is available to upgrade the CAL, but it cannot upgrade the license and the old license is not valid.

// Or:

//    A Client New License Request message was received earlier, and the license server is available to issue a new CAL, but the server was not able to issue a new license.

// In this case, if the grace period has not been exceeded, the server responds as if the client presented a valid license by sending a license error message with an error code of STATUS_VALID_CLIENT (0x00000007) and a state transition code of ST_NO_TRANSITION (0x00000002), ending the licensing protocol.

// If the server's grace period has been exceeded, it sends a license error message with an error code of ERR_INVALID_CLIENT (0x00000008) and a state transition of ST_TOTAL_ABORT (0x00000001). The licensing protocol is aborted.

// Case 4: The CAL (LicenseInfo BLOB) in the Client License Information message received by the server required an upgrade, and the CAL is valid and the license server available, but the license server cannot upgrade the license. In this case, the terminal server sends the Server Upgrade License message and returns the old license to the client.

// Case 5: A Client License Information message was received earlier by the server; the CAL (LicenseInfo BLOB) in the message required an upgrade; the license server can be contacted; and the old license is successfully upgraded. In this case, the terminal server returns the upgraded CAL in a Server Upgrade License message.

// Case 6: A Client New License Request message was received earlier, the license server was contacted, and it issued a new license. In this case, the terminal server sends the new license to the client in a Server New License message.

        // wVersion (2 bytes): A 16-bit unsigned integer that contains the platform
        // challenge version. This field MUST be set to 0x0100.

        // wClientType (2 bytes): A 16-bit unsigned integer that represents the
        // operating system type of the client and MAY contain one of following values.<15>

    // +-------------------------------------+--------------------------------+
    // | 0x0100 WIN32_PLATFORMCHALLENGE_TYPE | Win32 Platform Challenge Type. |
    // +-------------------------------------+--------------------------------+
    // | 0x0200 WIN16_PLATFORMCHALLENGE_TYPE | Win16 Platform Challenge Type. |
    // +-------------------------------------+--------------------------------+
    // | 0x0300 WINCE_PLATFORMCHALLENGE_TYPE | WinCE Platform Challenge Type. |
    // +-------------------------------------+--------------------------------+
    // | 0xFF00 OTHER_PLATFORMCHALLENGE_TYPE | Other Platform Challenge Type. |
    // +-------------------------------------+--------------------------------+

    // wLicenseDetailLevel (2 bytes): A 16-bit unsigned integer. This field
    // represents the capability of the client to handle license data. RDP
    // version 5.0 and later clients SHOULD advertise support for large (6.5 KB
    // or higher) licenses by setting the detail level to LICENSE_DETAIL_DETAIL
    // (0x0003). The following table lists valid values for this field.


#endif
