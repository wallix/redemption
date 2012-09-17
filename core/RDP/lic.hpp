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

    // 2.2.2.4 Server Platform Challenge (SERVER_PLATFORM_CHALLENGE)
    // -------------------------------------------------------------

    // The Server Platform Challenge packet is sent from the server to
    // the client after receiving the Client New License Request (section
    // 2.2.2.2) or certain cases of Client License Information (section
    // 2.2.2.3). For more information on Client License Information and
    // when Server Platform Challenge is sent, see Processing Client
    // License Information (section 3.2.5.3).


    //    ConnectFlags (4 bytes): Reserved.

    //    EncryptedPlatformChallenge (variable): A Licensing Binary BLOB<13>
    // structure (see [MS-RDPBCGR] section 2.2.1.12.1.2). This BLOB contains
    // the encrypted server platform challenge data. The server platform 
    // challenge data is a random string generated by the server and is 
    // encrypted with the licensing encryption key (see section 5.1.3) using 
    // RC4 (for instructions on how to perform the encryption, see section 5.1.4).

    //    MACData (16 bytes): An array of 16 bytes containing an MD5 digest (MAC)
    // generated over the unencrypted platform challenge BLOB. For instructions on
    // how to generate this message digest, see section 5.1.6; for a description of
    // how the client uses the MACData field to verify the integrity of the platform
    // challenge BLOB, see section 3.1.5.1.


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

    //  2.2.2.7 Server New License (SERVER_NEW_LICENSE)
    //  -----------------------------------------------

    //  The Server New License message is sent from the server to the client when 
    // a new license is issued to the client. The structure and the content of this
    // message are the same as the Server Upgrade License message.

    //    EncryptedLicenseInfo (variable): A LICENSE_BINARY_BLOB structure 
    // (as specified in [MS-RDPBCGR] section 2.2.1.12.1.2) 
    // of wBlobType BB_ENCRYPTED_DATA_BLOB (0x0009).<16> This BLOB contains 
    // the encrypted New License Information (section 2.2.2.6.1) packet and
    // is encrypted with the licensing encryption key (see section 5.1.3),
    // using RC4 (for instructions on how to perform the encryption, 
    // see section 5.1.4).

    // 2.2.2.6.1 New License Information (NEW_LICENSE_INFO)
    // -------------------------------------------------------

    //    The New License Information packet contains the actual client license
    // and associated indexing information. The client stores the license in its
    // license store using the indexing information, and uses it in subsequent 
    // connections. The dwVersion, pbScope, pbCompanyName, and pbProductId fields
    // are used by the client to index the licenses in the client's license store.

    //    EncryptedLicenseInfo::dwVersion (4 bytes): The content and format of this field are the same 
    // as the dwVersion field of the Product Information (section 2.2.2.1.1) structure.
    // it's a 32-bit unsigned integer that contains the license version information. The high-order word
    // contains the major version of the operating system on which the terminal server is running, while
    // the low-order word contains the minor version.<6>

    //     EncryptedLicenseInfo::cbScope (4 bytes): A 32-bit unsigned integer that contains the number of 
    // bytes in the string contained in the pbScope field.

    //     EncryptedLicenseInfo::pbScope (variable): Contains the NULL-terminated ANSI character set string
    // giving the name of the issuer of this license. For example, for licenses issued
    // by TailSpin Toys, this field contains the string "TailSpin Toys".

    //     EncryptedLicenseInfo::cbCompanyName (4 bytes): The content and format of this field are the same as
    // the cbCompanyName field of the Product Information structure.

    //     EncryptedLicenseInfo::pbCompanyName (variable): The content and format of this field are the same as
    // the pbCompanyName field of the Product Information structure.

    //     EncryptedLicenseInfo::cbProductId (4 bytes): The content and format of this field are the same as 
    // the cbProductId field of the Product Information structure.

    //     EncryptedLicenseInfo::pbProductId (variable): The content and format of this field are the same as
    // the pbProductId field of the Product Information structure.

    //     EncryptedLicenseInfo::cbLicenseInfo (4 bytes): A 32-bit unsigned integer that contains the number
    // of bytes of binary data in the pbLicenseInfo field.

    //     EncryptedLicenseInfo::pbLicenseInfo (variable): This field contains the CAL issued to the client by
    // the license server. This license consists of an X.509 certificate chain generated 
    // by the license server. The binary data contained in this field is opaque to the 
    // client. The client sends this information back to the server in the Client License 
    // Information message.

    // ---------------------------------------------------------

    //    MACData (16 bytes): An array of 16 bytes containing an MD5 digest
    // (Message Authentication Code) generated over the unencrypted New License
    // Information structure. For instructions on how to generate this message
    // digest, see section 5.1.6; for a description of how the server uses the
    // MACData field to verify the integrity of the New License Information packet,
    // see section 3.1.5.1.


    // 4.6 SERVER NEW LICENSE
    // ----------------------

    //    The server sends the license to the client in the Server New License message. 
    // See sections 2.2.2.7 and 3.2.5.7 for more information.

    //    00000000:  03 03 07 08 09 00 ef 07-db a3 13 30 79 a3 cd 9e  ...........0y...
    //    00000010:  48 f4 8f 06 37 1b 45 dd-60 a9 2e 29 26 bf c1 96  H...7.E.`..)&...
    //    00000020:  5e 07 93 9d f2 2d 3e a3-3a ff d5 6d f5 85 30 28  ^....->.:..m..0(
    //    00000030:  e1 46 fd 56 d1 20 41 33-94 88 0c 27 23 a0 61 38  .F.V. A3...'#.a8
    //    00000040:  60 db 86 d6 ce 2c cd 40-39 55 23 39 12 b9 fd c2  `....,.@9U#9....
    //    00000050:  8d 58 0a 37 33 42 5c 61-d7 c8 a0 11 66 e2 45 ba  .X.73B\a....f.E.
    //    00000060:  41 39 ea 85 2a 6e 7a b3-e7 27 75 fc 4d c0 fb 0d  A9..*nz..'u.M...
    //    00000070:  e8 67 90 b3 3a 40 f0 15-8a 15 8e 2c 99 0f 1c bd  .g..:@.....,....
    //    00000080:  d2 08 66 51 9e 6a e6 2c-f7 1f d0 c0 8e 89 76 64  ..fQ.j.,......vd
    //    00000090:  18 58 a1 94 bd ce b1 2d-96 ab 53 cf f8 bf d0 c9  .X.....-..S.....
    //    000000A0:  c0 2e e6 a4 0b 50 31 4a-4e d8 47 4b af b8 21 78  .....P1JN.GK..!x
    //    000000B0:  bf 09 ac 7f 2d 2d 88 f6-d8 c7 45 33 9f ac 69 f5  ....--....E3..i.
    //    000000C0:  88 9d 5c 6e c9 d0 ca 8c-bc a9 d6 07 36 ed 40 95  ..\n........6.@.
    //    000000D0:  8a c1 3f 04 41 b3 c9 b3-18 9d 33 1b 04 55 cd 41  ..?.A.....3..U.A
    //    000000E0:  df 19 e1 cd a0 a4 35 6e-b7 0a f3 ec 48 10 4f 28  ......5n....H.O(
    //    000000F0:  c6 35 f3 9b a2 d5 f7 58-03 4d 9a 16 34 fb 96 0c  .5.....X.M..4...
    //    00000100:  d5 3a ae 52 1b 2f 1f 1f-31 b2 d9 14 3b 73 0f e3  .:.R./..1...;s..
    //    00000110:  04 e0 a5 52 89 68 ba 0f-99 9d 24 a6 f3 e8 9f cc  ...R.h....$.....
    //    00000120:  d2 44 9f 08 8b 0a 24 89-f7 c9 07 0d 25 07 ed 3e  .D....$.....%..>
    //    00000130:  75 21 19 65 dc 98 41 9d-05 12 18 88 86 16 43 49  u!.e..A.......CI
    //    00000140:  29 f2 e8 26 16 1e ce cd-32 e7 36 74 51 27 fd a2  )..&....2.6tQ'..
    //    00000150:  a9 62 57 60 28 e4 64 02-06 6b ff 01 ab c5 1c 25  .bW`(.d..k.....%
    //    00000160:  98 07 e1 40 ad 19 b7 68-66 12 4e 80 bc 83 d2 de  ...@...hf.N.....
    //    00000170:  cb 7e c2 32 c7 b8 4d d6-7d dd 63 a9 95 45 c1 90  .~.2..M.}.c..E..
    //    00000180:  c7 99 3c 0a 24 62 fc 24-15 db d3 d2 9b 5d 78 04  ..<.$b.$.....]x.
    //    00000190:  78 d5 40 1d e3 4e e8 30-9f 56 91 71 00 86 2c 6a  x.@..N.0.V.q..,j
    //    000001A0:  b2 78 ec 70 d9 71 e6 aa-b1 ad 18 f9 a6 84 b7 4b  .x.p.q.........K
    //    000001B0:  5f 32 b8 e3 c7 84 ef 37-fe ae 99 b5 f2 34 84 82  _2.....7.....4..
    //    000001C0:  4a b3 d0 7c 5e 25 71 89-8b 7d 6f 5f 96 7a 1d 84  J..|^%q..}o_.z..
    //    000001D0:  96 56 34 30 ce 09 d5 00-a8 ac 15 72 21 c4 71 57  .V40.......r!.qW
    //    000001E0:  e5 2a 3d df 82 b8 b8 63-dc 3f 2e 99 6c c3 e3 fd  .*=....c.?..l...
    //    000001F0:  92 e0 26 e1 27 b8 04 71-b0 a8 d1 df 7e 24 23 b9  ..&.'..q....~$#.
    //    00000200:  82 01 77 dc 8f 77 54 e6-93 c8 6c 66 87 b7 aa 9d  ..w..wT...lf....
    //    00000210:  66 d4 c6 2f 5e 9e e1 cf-db b2 74 0e ea a5 e0 f7  f../^.....t.....
    //    00000220:  00 f1 76 f7 45 2c f8 a9-3b d9 81 59 52 0f fe d9  ..v.E,..;..YR...
    //    00000230:  28 02 59 82 39 51 6e b9-ac f9 6a 48 73 6f 2c 4d  (.Y.9Qn...jHso,M
    //    00000240:  7b c0 bf be 69 ae 0e dc-8b e6 d8 9f 66 30 1e 45  {...i.......f0.E
    //    00000250:  1d 85 23 eb a8 02 b5 ba-c2 fd a1 ff c5 55 2b a0  ..#..........U+.
    //    00000260:  f7 5b 24 ee 81 d8 e1 b8-02 06 85 6e 41 5a b8 07  .[$........nAZ..
    //    00000270:  ff 65 db b4 59 89 71 95-d5 0c 2a 67 4d 57 fd 4a  .e..Y.q...*gMW.J
    //    00000280:  e8 07 02 42 20 d9 f1 c6-d5 4c 53 b0 32 68 c0 dc  ...B ....LS.2h..
    //    00000290:  d7 5d 8f ec 24 29 00 4f-46 8d d2 99 b2 f4 06 99  .]..$).OF.......
    //    000002A0:  9a a6 31 f1 49 16 fe 94-bb 8e 15 55 06 93 16 a3  ..1.I......U....
    //    000002B0:  2d 10 b7 b1 cf 61 78 af-93 66 5a 75 5e 97 c0 97  -....ax..fZu^...
    //    000002C0:  4c ba a9 50 ac 1b d6 92-2a ac 0a 21 12 9e 4a f0  L..P....*..!..J.
    //    000002D0:  40 39 4b e5 78 88 86 17-b9 eb a0 33 8a 9a fc 7c  @9K.x......3...|
    //    000002E0:  91 16 d7 52 ec 05 7e 4e-90 78 5e 45 4a dd f6 f4  ...R..~N.x^EJ...
    //    000002F0:  2e 68 f7 8e fc 60 95 aa-6a 07 9c ea ce c1 d9 55  .h...`..j......U
    //    00000300:  3a 78 54 9a 2a 5f 47 87-18 4a 8c 6c 34 f5 b8 e2  :xT.*_G..J.l4...
    //    00000310:  84 36 ef 0d 2e 9d 42 d9-ff 56 e2 87 0b 2f 4d 0e  .6....B..V.../M.
    //    00000320:  c0 60 35 06 9f 61 9e 4e-7b 49 41 b4 fa 04 10 bd  .`5..a.N{IA.....
    //    00000330:  f6 ad 02 d9 7c ba 06 68-bb a7 a6 8a ab ab b1 2d  ....|..h.......-
    //    00000340:  69 2a f1 c6 7b 1b 71 b9-d0 91 82 6f a8 3c e1 a3  i*..{.q....o.<..
    //    00000350:  23 3d 4e 48 74 e5 c9 c5-95 31 ad e7 a9 db 35 cd  #=NHt....1....5.
    //    00000360:  02 08 2c 29 5f f9 17 86-69 8f 13 d1 ca 83 fc ac  ..,)_...i.......
    //    00000370:  55 cf 5a e6 45 af e5 bb-e7 b5 53 4e f0 63 fc 9a  U.Z.E.....SN.c..
    //    00000380:  49 f6 45 93 c6 bf d5 b3-25 e2 93 b0 a6 a7 14 80  I.E.....%.......
    //    00000390:  6d b2 03 15 6a ad e8 25-f1 80 d4 ba 9a 88 bc 56  m...j..%.......V
    //    000003A0:  14 7a 4d ad c3 24 3f 4d-35 8b f6 59 5b fd c9 32  .zM..$?M5..Y[..2
    //    000003B0:  1d f5 a5 53 b5 fb ba 83-29 0b 9c 62 9b 56 4b 44  ...S....)..b.VKD
    //    000003C0:  bc cc 19 59 7c 0b 74 d9-04 28 b3 67 ab 82 36 39  ...Y|.t..(.g..69
    //    000003D0:  55 5f 7c ed 84 c1 16 d3-9e 9c 90 9d 55 bc 3e b9  U_|.........U.>.
    //    000003E0:  63 12 f2 26 6d d7 cc 4f-01 a2 0b d9 66 60 ad ed  c..&m..O....f`..
    //    000003F0:  2e bd be 28 5f 4a 33 c8-e8 d4 a6 23 8a fd 66 f5  ...(_J3....#..f.
    //    00000400:  28 90 81 27 a9 44 93 68-57 44 5e ba 90 12 03 15  (..'.D.hWD^.....
    //    00000410:  2f 69 80 55 e8 32 63 88-30 85 50 9b b6 bc bb c6  /i.U.2c.0.P.....
    //    00000420:  fe a2 e7 32 9d 3d 7d e2-31 93 a6 4e a0 dc 11 44  ...2.=}.1..N...D
    //    00000430:  d2 93 32 94 1e c6 4c 28-de a2 a6 0d 14 02 74 8a  ..2...L(......t.
    //    00000440:  84 2a 03 67 35 1d 66 3e-e9 68 4a b2 92 1a 69 48  .*.g5.f>.hJ...iH
    //    00000450:  bd 23 fd 70 5a fd fe 74-39 c5 fa 11 ac 04 c9 94  .#.pZ..t9.......
    //    00000460:  fc 12 2e 06 04 61 8e 32-f4 f8 3b d1 d8 09 b3 e4  .....a.2..;.....
    //    00000470:  ac 0f 3e 92 f7 75 0b 32-9b d4 8a 13 99 6a 26 77  ..>..u.2.....j&w
    //    00000480:  9f 34 08 a8 eb b3 3e 2a-5b 4a 44 f9 21 89 2a 09  .4....>*[JD.!.*.
    //    00000490:  9c c7 0d 2a d8 d6 27 30-25 39 84 14 11 47 ff 60  ...*..'0%9...G.`
    //    000004A0:  e4 7c c3 41 fd d5 34 dd-b1 1c f2 eb b1 67 04 fa  .|.A..4......g..
    //    000004B0:  fd 65 5b 20 6e 28 75 a9-74 8e c4 2f f7 b2 f5 9f  .e[ n(u.t../....
    //    000004C0:  13 44 ff b0 f0 68 b0 69-15 a6 16 a8 ac c3 06 14  .D...h.i........
    //    000004D0:  8e 51 99 a9 4c 19 d1 25-34 b5 79 c2 a7 bf d8 3d  .Q..L..%4.y....=
    //    000004E0:  2d 4c 33 ac 1b 6c af 10-42 41 14 02 e6 87 2b e9  -L3..l..BA....+.
    //    000004F0:  ec c6 b1 eb 97 d4 35 49-97 fc e2 73 f9 98 46 7c  ......5I...s..F|
    //    00000500:  f6 17 2d b5 43 07 8b 19-95 9b 65 d3 05 7e b0 68  ..-.C.....e..~.h
    //    00000510:  0d 6e 4b 60 ad 5c 47 6e-37 fd 3f 60 43 da b2 34  .nK`.\Gn7.?`C..4
    //    00000520:  00 d6 9c 6d 46 7f 41 e2-c1 1a d0 53 72 81 0b 3f  ...mF.A....Sr..?
    //    00000530:  77 e1 bc cc 09 0f a1 1d-73 8c ac a4 48 90 80 a8  w.......s...H...
    //    00000540:  50 63 6a b7 76 91 91 2f-1a 5e 83 80 e6 ae 66 77  Pcj.v../.^....fw
    //    00000550:  44 e0 0f 14 70 c9 d3 91-e8 d2 c4 89 a8 45 c0 3d  D...p........E.=
    //    00000560:  bd 09 58 e0 cd e6 5c 9e-02 94 d2 be df 94 35 f7  ..X...\.......5.
    //    00000570:  67 96 75 88 08 59 d9 19-21 da d0 a2 74 2d 22 87  g.u..Y..!...t-".
    //    00000580:  37 27 6e 58 dc 8e 9f 50-d5 62 f3 4a a4 b2 fb f9  7'nX...P.b.J....
    //    00000590:  3e d5 da 57 56 5c cb 0e-d1 62 4f ea 42 4a 62 b2  >..WV\...bO.BJb.
    //    000005A0:  4e 1c d1 cc 24 1b dc ac-d4 b0 2f 5d 62 87 56 3d  N...$...../]b.V=
    //    000005B0:  e4 03 ae 4a 7e 7d 05 fe-85 33 da 5f 36 cb 56 a1  ...J~}...3._6.V.
    //    000005C0:  14 80 63 26 75 3a c2 1f-9c dd 8a d6 f8 a9 1a f6  ..c&u:..........
    //    000005D0:  c2 57 e0 7b 80 55 d5 12-f1 b4 e7 1d 95 68 02 f1  .W.{.U.......h..
    //    000005E0:  19 ff 74 72 32 db 6c c9-a0 2d 69 fc c0 e8 27 11  ..tr2.l..-i...'.
    //    000005F0:  b8 e0 f5 83 60 3f b4 94-e5 9d b8 fd c5 9e 50 76  ....`?........Pv
    //    00000600:  92 c7 33 6b e0 7e 9b 2c-3b 27 d8 a6 da b3 a1 45  ..3k.~.,;'.....E
    //    00000610:  ef 3b 3d 76 1d 5a 43 a4-e1 82 5f 7d 0b 10 28 1c  .;=v.ZC..._}..(.
    //    00000620:  b9 8f 2b cd f9 c5 93 e3-65 a5 5b 50 a9 07 16 b3  ..+.....e.[P....
    //    00000630:  45 4a 69 fc 58 12 7b 58-34 9f 6f 5e 7a c2 9f 7b  EJi.X.{X4.o^z..{
    //    00000640:  cf 62 62 6c 1e 7d d9 72-ca 98 1e d4 e5 c7 98 27  .bbl.}.r.......'
    //    00000650:  e3 9f c5 3c 90 9e 26 ed-7e 38 2b cf c4 99 f5 b4  ...<..&.~8+.....
    //    00000660:  78 48 7c 08 99 bc 80 47-b1 c5 58 60 17 84 11 f8  xH|....G..X`....
    //    00000670:  70 a1 26 95 94 77 f4 8d-1d 47 96 df 95 1d 97 37  p.&..w...G.....7
    //    00000680:  5a 12 fb 3f cd a0 fb ac-61 62 1f ee dc 6d 2c 45  Z..?....ab...m,E
    //    00000690:  5f c0 80 7d 99 62 38 f7-6d 88 d5 e2 24 9d ed a7  _..}.b8.m...$...
    //    000006A0:  d5 e7 1c 8a 75 9b 67 0c-64 fe e7 e2 e9 a0 94 60  ....u.g.d......`
    //    000006B0:  26 46 d8 c9 59 43 17 85-07 32 8b 0c 5a 1e 35 48  &F..YC...2..Z.5H
    //    000006C0:  46 e5 44 5b c7 07 cd 30-97 80 fa f9 eb 0d af af  F.D[...0........
    //    000006D0:  3e be 27 43 8e 4a f2 60-eb 2b 4d 11 9a e1 d7 59  >.'C.J.`.+M....Y
    //    000006E0:  78 29 26 3e 9b da 61 15-ab e8 6f 81 73 ac 9f 43  x)&>..a...o.s..C
    //    000006F0:  c6 b1 81 73 36 4a 1e 57-2b d9 7a 06 4f b6 37 11  ...s6J.W+.z.O.7.
    //    00000700:  e6 c6 90 41 a4 a6 b7 3a-7e c9 ce 50 2f 8c 07 db  ...A...:~..P/...
    //    00000710:  9a 19 38 51 35 50 f3 09-33 20 62 36 a8 6c 8e f2  ..8Q5P..3 b6.l..
    //    00000720:  56 8c 82 d3 fa 16 b5 3d-74 28 dc 2a e5 ae ab 77  V......=t(.*...w
    //    00000730:  e5 22 3d 69 92 b8 56 c5-06 6b 8b da 46 18 af 93  ."=i..V..k..F...
    //    00000740:  a2 0c b5 d3 d6 94 4a 29-fd 4b 48 48 a1 73 bc de  ......J).KHH.s..
    //    00000750:  cb 3a 35 27 d5 40 3a 1d-5b e3 62 a5 22 a2 7b b5  .:5'.@:.[.b.".{.
    //    00000760:  6a f0 5e 0a 96 5c e8 3b-41 58 a1 d5 11 2c 36 9e  j.^..\.;AX...,6.
    //    00000770:  e5 2b c9 fb 1b 37 d0 8e-cf c2 01 6f b6 21 96 9c  .+...7.....o.!..
    //    00000780:  14 2f 76 19 b0 6a 9d ef-69 fd d0 03 d9 eb b7 86  ./v..j..i.......
    //    00000790:  76 20 d3 20 6a d8 cb c8-9e b7 82 28 b2 25 a7 a2  v . j......(.%..
    //    000007A0:  6b fd 60 b0 11 09 53 5f-79 6b 72 d7 1b 21 73 f7  k.`...S_ykr..!s.
    //    000007B0:  21 d5 5c c4 e5 52 82 73-1f 9c 95 e1 21 bf 12 67  !.\..R.s....!..g
    //    000007C0:  c9 41 e5 64 c4 d4 f9 a2-9b 29 df e5 a4 f3 b4 69  .A.d.....).....i
    //    000007D0:  6d 2c b0 42 e3 e6 25 a7-8f f5 12 99 8c bf bb d8  m,.B..%.........
    //    000007E0:  a8 23 db 8e ec 37 b0 8e-4f ed 67 aa 3e f6 24 56  .#...7..O.g.>.$V
    //    000007F0:  96 e3 d4 bc 25 0b 56 ed-e8 bf d6 13 a0 f5 80 4a  ....%.V........J
    //    00000800:  e5 ff 85 16 fa cb 1f                             .......

    //    0x00: LICENSE_PREAMBLE    (4 bytes)
    //       03 -> LICENSE_PREAMBLE::bMsgType = SERVER_NEW_LICENSE
    //     
    //    03 -> LICENSE_PREAMBLE::bVersion = 3
    //     
    //    07 -\|
    //    08 -/ LICENSE_PREAMBLE::wMsgSize = 0x807 bytes
    //    0x04: EncryptedLicenseInfo (2 + 2 + 0x7ef = 0x7f3 bytes)
    //    09 -\|
    //    00 -/ EncryptedLicenseInfo::wBlobType = BB_ENCRYPTED_DATA_BLOB
    //     
    //    ef -\|
    //    07 -/ EncryptedLicenseInfo::wBlobLen = 0x7ef bytes
    //       
    //                            db a3 13 30 79 a3 cd 9e -\|
    //    48 f4 8f 06 37 1b 45 dd-60 a9 2e 29 26 bf c1 96 -|
    //    5e 07 93 9d f2 2d 3e a3-3a ff d5 6d f5 85 30 28 -|
    //    e1 46 fd 56 d1 20 41 33-94 88 0c 27 23 a0 61 38 -|
    //    60 db 86 d6 ce 2c cd 40-39 55 23 39 12 b9 fd c2 -|
    //    8d 58 0a 37 33 42 5c 61-d7 c8 a0 11 66 e2 45 ba -|
    //    41 39 ea 85 2a 6e 7a b3-e7 27 75 fc 4d c0 fb 0d -|
    //    e8 67 90 b3 3a 40 f0 15-8a 15 8e 2c 99 0f 1c bd -|
    //    d2 08 66 51 9e 6a e6 2c-f7 1f d0 c0 8e 89 76 64 -|
    //    18 58 a1 94 bd ce b1 2d-96 ab 53 cf f8 bf d0 c9 -|
    //    c0 2e e6 a4 0b 50 31 4a-4e d8 47 4b af b8 21 78 -|
    //    bf 09 ac 7f 2d 2d 88 f6-d8 c7 45 33 9f ac 69 f5 -|
    //    88 9d 5c 6e c9 d0 ca 8c-bc a9 d6 07 36 ed 40 95 -|
    //    8a c1 3f 04 41 b3 c9 b3-18 9d 33 1b 04 55 cd 41 -|
    //    df 19 e1 cd a0 a4 35 6e-b7 0a f3 ec 48 10 4f 28 -|
    //    c6 35 f3 9b a2 d5 f7 58-03 4d 9a 16 34 fb 96 0c -|
    //    d5 3a ae 52 1b 2f 1f 1f-31 b2 d9 14 3b 73 0f e3 -|
    //    04 e0 a5 52 89 68 ba 0f-99 9d 24 a6 f3 e8 9f cc -|
    //    d2 44 9f 08 8b 0a 24 89-f7 c9 07 0d 25 07 ed 3e -|
    //    75 21 19 65 dc 98 41 9d-05 12 18 88 86 16 43 49 -|
    //    29 f2 e8 26 16 1e ce cd-32 e7 36 74 51 27 fd a2 -|
    //    a9 62 57 60 28 e4 64 02-06 6b ff 01 ab c5 1c 25 -|
    //    98 07 e1 40 ad 19 b7 68-66 12 4e 80 bc 83 d2 de -|
    //    cb 7e c2 32 c7 b8 4d d6-7d dd 63 a9 95 45 c1 90 -|
    //    c7 99 3c 0a 24 62 fc 24-15 db d3 d2 9b 5d 78 04 -|
    //    78 d5 40 1d e3 4e e8 30-9f 56 91 71 00 86 2c 6a -|
    //    b2 78 ec 70 d9 71 e6 aa-b1 ad 18 f9 a6 84 b7 4b -|
    //    5f 32 b8 e3 c7 84 ef 37-fe ae 99 b5 f2 34 84 82 -|
    //    4a b3 d0 7c 5e 25 71 89-8b 7d 6f 5f 96 7a 1d 84 -|
    //    96 56 34 30 ce 09 d5 00-a8 ac 15 72 21 c4 71 57 -|
    //    e5 2a 3d df 82 b8 b8 63-dc 3f 2e 99 6c c3 e3 fd -|
    //    92 e0 26 e1 27 b8 04 71-b0 a8 d1 df 7e 24 23 b9 -|
    //    82 01 77 dc 8f 77 54 e6-93 c8 6c 66 87 b7 aa 9d -|
    //    66 d4 c6 2f 5e 9e e1 cf-db b2 74 0e ea a5 e0 f7 -|
    //    00 f1 76 f7 45 2c f8 a9-3b d9 81 59 52 0f fe d9 -|
    //    28 02 59 82 39 51 6e b9-ac f9 6a 48 73 6f 2c 4d -|
    //    7b c0 bf be 69 ae 0e dc-8b e6 d8 9f 66 30 1e 45 -|
    //    1d 85 23 eb a8 02 b5 ba-c2 fd a1 ff c5 55 2b a0 -|
    //    f7 5b 24 ee 81 d8 e1 b8-02 06 85 6e 41 5a b8 07 -|
    //    ff 65 db b4 59 89 71 95-d5 0c 2a 67 4d 57 fd 4a -|
    //    e8 07 02 42 20 d9 f1 c6-d5 4c 53 b0 32 68 c0 dc -|
    //    d7 5d 8f ec 24 29 00 4f-46 8d d2 99 b2 f4 06 99 -|
    //    9a a6 31 f1 49 16 fe 94-bb 8e 15 55 06 93 16 a3 -|
    //    2d 10 b7 b1 cf 61 78 af-93 66 5a 75 5e 97 c0 97 -|
    //    4c ba a9 50 ac 1b d6 92-2a ac 0a 21 12 9e 4a f0 -|
    //    40 39 4b e5 78 88 86 17-b9 eb a0 33 8a 9a fc 7c -|
    //    91 16 d7 52 ec 05 7e 4e-90 78 5e 45 4a dd f6 f4 -|
    //    2e 68 f7 8e fc 60 95 aa-6a 07 9c ea ce c1 d9 55 -|
    //    3a 78 54 9a 2a 5f 47 87-18 4a 8c 6c 34 f5 b8 e2 -|
    //    84 36 ef 0d 2e 9d 42 d9-ff 56 e2 87 0b 2f 4d 0e -|
    //    c0 60 35 06 9f 61 9e 4e-7b 49 41 b4 fa 04 10 bd -|
    //    f6 ad 02 d9 7c ba 06 68-bb a7 a6 8a ab ab b1 2d -|
    //    69 2a f1 c6 7b 1b 71 b9-d0 91 82 6f a8 3c e1 a3 -|
    //    23 3d 4e 48 74 e5 c9 c5-95 31 ad e7 a9 db 35 cd -|
    //    02 08 2c 29 5f f9 17 86-69 8f 13 d1 ca 83 fc ac -|
    //    55 cf 5a e6 45 af e5 bb-e7 b5 53 4e f0 63 fc 9a -|
    //    49 f6 45 93 c6 bf d5 b3-25 e2 93 b0 a6 a7 14 80 -|
    //    6d b2 03 15 6a ad e8 25-f1 80 d4 ba 9a 88 bc 56 -|
    //    14 7a 4d ad c3 24 3f 4d-35 8b f6 59 5b fd c9 32 -|
    //    1d f5 a5 53 b5 fb ba 83-29 0b 9c 62 9b 56 4b 44 -|
    //    bc cc 19 59 7c 0b 74 d9-04 28 b3 67 ab 82 36 39 -|
    //    55 5f 7c ed 84 c1 16 d3-9e 9c 90 9d 55 bc 3e b9 -|
    //    63 12 f2 26 6d d7 cc 4f-01 a2 0b d9 66 60 ad ed -|
    //    2e bd be 28 5f 4a 33 c8-e8 d4 a6 23 8a fd 66 f5 -|
    //    28 90 81 27 a9 44 93 68-57 44 5e ba 90 12 03 15 -|
    //    2f 69 80 55 e8 32 63 88-30 85 50 9b b6 bc bb c6 -|
    //    fe a2 e7 32 9d 3d 7d e2-31 93 a6 4e a0 dc 11 44 -|
    //    d2 93 32 94 1e c6 4c 28-de a2 a6 0d 14 02 74 8a -|
    //    84 2a 03 67 35 1d 66 3e-e9 68 4a b2 92 1a 69 48 -|
    //    bd 23 fd 70 5a fd fe 74-39 c5 fa 11 ac 04 c9 94 -|
    //    fc 12 2e 06 04 61 8e 32-f4 f8 3b d1 d8 09 b3 e4 -|
    //    ac 0f 3e 92 f7 75 0b 32-9b d4 8a 13 99 6a 26 77 -|
    //    9f 34 08 a8 eb b3 3e 2a-5b 4a 44 f9 21 89 2a 09 -|
    //    9c c7 0d 2a d8 d6 27 30-25 39 84 14 11 47 ff 60 -|
    //    e4 7c c3 41 fd d5 34 dd-b1 1c f2 eb b1 67 04 fa -|
    //    fd 65 5b 20 6e 28 75 a9-74 8e c4 2f f7 b2 f5 9f -|
    //    13 44 ff b0 f0 68 b0 69-15 a6 16 a8 ac c3 06 14 -|
    //    8e 51 99 a9 4c 19 d1 25-34 b5 79 c2 a7 bf d8 3d -|
    //    2d 4c 33 ac 1b 6c af 10-42 41 14 02 e6 87 2b e9 -|
    //    ec c6 b1 eb 97 d4 35 49-97 fc e2 73 f9 98 46 7c -|
    //    f6 17 2d b5 43 07 8b 19-95 9b 65 d3 05 7e b0 68 -|
    //    0d 6e 4b 60 ad 5c 47 6e-37 fd 3f 60 43 da b2 34 -|
    //    00 d6 9c 6d 46 7f 41 e2-c1 1a d0 53 72 81 0b 3f -|
    //    77 e1 bc cc 09 0f a1 1d-73 8c ac a4 48 90 80 a8 -|
    //    50 63 6a b7 76 91 91 2f-1a 5e 83 80 e6 ae 66 77 -|
    //    44 e0 0f 14 70 c9 d3 91-e8 d2 c4 89 a8 45 c0 3d -|
    //    bd 09 58 e0 cd e6 5c 9e-02 94 d2 be df 94 35 f7 -|
    //    67 96 75 88 08 59 d9 19-21 da d0 a2 74 2d 22 87 -|
    //    37 27 6e 58 dc 8e 9f 50-d5 62 f3 4a a4 b2 fb f9 -|
    //    3e d5 da 57 56 5c cb 0e-d1 62 4f ea 42 4a 62 b2 -|
    //    4e 1c d1 cc 24 1b dc ac-d4 b0 2f 5d 62 87 56 3d -|
    //    e4 03 ae 4a 7e 7d 05 fe-85 33 da 5f 36 cb 56 a1 -|
    //    14 80 63 26 75 3a c2 1f-9c dd 8a d6 f8 a9 1a f6 -|
    //    c2 57 e0 7b 80 55 d5 12-f1 b4 e7 1d 95 68 02 f1 -|
    //    19 ff 74 72 32 db 6c c9-a0 2d 69 fc c0 e8 27 11 -|
    //    b8 e0 f5 83 60 3f b4 94-e5 9d b8 fd c5 9e 50 76 -|
    //    92 c7 33 6b e0 7e 9b 2c-3b 27 d8 a6 da b3 a1 45 -|
    //    ef 3b 3d 76 1d 5a 43 a4-e1 82 5f 7d 0b 10 28 1c -|
    //    b9 8f 2b cd f9 c5 93 e3-65 a5 5b 50 a9 07 16 b3 -|
    //    45 4a 69 fc 58 12 7b 58-34 9f 6f 5e 7a c2 9f 7b -|
    //    cf 62 62 6c 1e 7d d9 72-ca 98 1e d4 e5 c7 98 27 -|
    //    e3 9f c5 3c 90 9e 26 ed-7e 38 2b cf c4 99 f5 b4 -|
    //    78 48 7c 08 99 bc 80 47-b1 c5 58 60 17 84 11 f8 -|
    //    70 a1 26 95 94 77 f4 8d-1d 47 96 df 95 1d 97 37 -|
    //    5a 12 fb 3f cd a0 fb ac-61 62 1f ee dc 6d 2c 45 -|
    //    5f c0 80 7d 99 62 38 f7-6d 88 d5 e2 24 9d ed a7 -|
    //    d5 e7 1c 8a 75 9b 67 0c-64 fe e7 e2 e9 a0 94 60 -|
    //    26 46 d8 c9 59 43 17 85-07 32 8b 0c 5a 1e 35 48 -|
    //    46 e5 44 5b c7 07 cd 30-97 80 fa f9 eb 0d af af -|
    //    3e be 27 43 8e 4a f2 60-eb 2b 4d 11 9a e1 d7 59 -|
    //    78 29 26 3e 9b da 61 15-ab e8 6f 81 73 ac 9f 43 -|
    //    c6 b1 81 73 36 4a 1e 57-2b d9 7a 06 4f b6 37 11 -|
    //    e6 c6 90 41 a4 a6 b7 3a-7e c9 ce 50 2f 8c 07 db -|
    //    9a 19 38 51 35 50 f3 09-33 20 62 36 a8 6c 8e f2 -|
    //    56 8c 82 d3 fa 16 b5 3d-74 28 dc 2a e5 ae ab 77 -|
    //    e5 22 3d 69 92 b8 56 c5-06 6b 8b da 46 18 af 93 -|
    //    a2 0c b5 d3 d6 94 4a 29-fd 4b 48 48 a1 73 bc de -|
    //    cb 3a 35 27 d5 40 3a 1d-5b e3 62 a5 22 a2 7b b5 -|
    //    6a f0 5e 0a 96 5c e8 3b-41 58 a1 d5 11 2c 36 9e -|
    //    e5 2b c9 fb 1b 37 d0 8e-cf c2 01 6f b6 21 96 9c -|
    //    14 2f 76 19 b0 6a 9d ef-69 fd d0 03 d9 eb b7 86 -|
    //    76 20 d3 20 6a d8 cb c8-9e b7 82 28 b2 25 a7 a2 -|
    //    6b fd 60 b0 11 09 53 5f-79 6b 72 d7 1b 21 73 f7 -|
    //    21 d5 5c c4 e5 52 82 73-1f 9c 95 e1 21 bf 12 67 -|
    //    c9 41 e5 64 c4 d4 f9 a2-9b 29 df e5 a4 f3 b4 69 -|
    //    6d 2c b0 42 e3 e6 25 a7-8f f5 12 99 8c bf bb d8 -|
    //    a8 23 db 8e ec 37 b0 8e-4f ed 67 aa 3e f6 24 56 -|
    //    96 e3 d4 bc 25 0b 56                            -/ 
    //    EncryptedLicenseInfo::pBlob

    //    The decrypted LicenseInfo for the above data is 
    //    00 -\|
    //    00 -|
    //    06 -|
    //    00 -/ LicenseInfo::dwVersion = 0x060000
    //     
    //    0e -\|
    //    00 -|
    //    00 -|
    //    00 -/ LicenseInfo::cbScope

    //    6d 69 63 72 6f 73 6f 66 -\|
    //    74 2e 63 6f 6d 00       -/ LicenseInfo::pbScope
    //     
    //    2c -\|
    //    00 -|
    //    00 -|
    //    00 -/ LicenseInfo::cbCompanyName

    //    4d 00 69 00 63 00 72 00 -\|
    //    6f 00 73 00 6f 00 66 00 -|
    //    74 00 20 00 43 00 6f 00 -|
    //    72 00 70 00 6f 00 72 00 -|
    //    61 00 74 00 69 00 6f 00 -|
    //    6e 00 00 00             -/ LicenseInfo::pbCompanyName
    //     
    //    08 -\|
    //    00 -|
    //    00 -|
    //    00 -/ LicenseInfo::cbProductId

    //    41 00 30 00 32 00 00 00 ->LicenseInfo::pbProductId
    //     
    //    99 -\|
    //    07 -|
    //    00 -|
    //    00 -/ LicenseInfo::cbLicenseInfo

    //                      30 82 07 95 06 09 2a 86 48 86 -\|
    //    f7 0d 01 07 02 a0 82 07-86 30 82 07 82 02 01 01 -|
    //    31 00 30 0b 06 09 2a 86-48 86 f7 0d 01 07 01 a0 -|
    //    82 07 6a 30 82 02 f1 30-82 01 dd a0 03 02 01 02 -|
    //    02 08 01 9e 27 4d 68 ac-ed 20 30 09 06 05 2b 0e -|
    //    03 02 1d 05 00 30 32 31-30 30 13 06 03 55 04 03 -|
    //    1e 0c 00 52 00 4f 00 44-00 45 00 4e 00 54 30 19 -|
    //    06 03 55 04 07 1e 12 00-57 00 4f 00 52 00 4b 00 -|
    //    47 00 52 00 4f 00 55 00-50 30 1e 17 0d 37 30 30 -|
    //    35 33 30 31 30 33 36 31-38 5a 17 0d 34 39 30 35 -|
    //    33 30 31 30 33 36 31 38-5a 30 32 31 30 30 13 06 -|
    //    03 55 04 03 1e 0c 00 52-00 4f 00 44 00 45 00 4e -|
    //    00 54 30 19 06 03 55 04-07 1e 12 00 57 00 4f 00 -|
    //    52 00 4b 00 47 00 52 00-4f 00 55 00 50 30 82 01 -|
    //    22 30 0d 06 09 2a 86 48-86 f7 0d 01 01 01 05 00 -|
    //    03 82 01 0f 00 30 82 01-0a 02 82 01 01 00 88 ad -|
    //    7c 8f 8b 82 76 5a bd 8f-6f 62 18 e1 d9 aa 41 fd -|
    //    ed 68 01 c6 34 35 b0 29-04 ca 4a 4a 1c 7e 80 14 -|
    //    f7 8e 77 b8 25 ff 16 47-6f bd e2 34 3d 2e 02 b9 -|
    //    53 e4 33 75 ad 73 28 80-a0 4d fc 6c c0 22 53 1b -|
    //    2c f8 f5 01 60 19 7e 79-19 39 8d b5 ce 39 58 dd -|
    //    55 24 3b 55 7b 43 c1 7f-14 2f b0 64 3a 54 95 2b -|
    //    88 49 0c 61 2d ac f8 45-f5 da 88 18 5f ae 42 f8 -|
    //    75 c7 26 6d b5 bb 39 6f-cc 55 1b 32 11 38 8d e4 -|
    //    e9 44 84 11 36 a2 61 76-aa 4c b4 e3 55 0f e4 77 -|
    //    8e de e3 a9 ea b7 41 94-00 58 aa c9 34 a2 98 c6 -|
    //    01 1a 76 14 01 a8 dc 30-7c 77 5a 20 71 5a a2 3f -|
    //    af 13 7e e8 fd 84 a2 5b-cf 25 e9 c7 8f a8 f2 8b -|
    //    84 c7 04 5e 53 73 4e 0e-89 a3 3c e7 68 5c 24 b7 -|
    //    80 53 3c 54 c8 c1 53 aa-71 71 3d 36 15 d6 6a 9d -|
    //    7d de ae f9 e6 af 57 ae-b9 01 96 5d e0 4d cd ed -|
    //    c8 d7 f3 01 03 38 10 be-7c 42 67 01 a7 23 02 03 -|
    //    01 00 01 a3 13 30 11 30-0f 06 03 55 1d 13 04 08 -|
    //    30 06 01 01 ff 02 01 00-30 09 06 05 2b 0e 03 02 -|
    //    1d 05 00 03 82 01 01 00-70 db 21 2b 84 9a 7a c3 -|
    //    b1 68 fa c0 00 8b 71 ab-43 9f b6 7b b7 1f 20 83 -|
    //    ac 0a b5 0e ad b6 36 ef-65 17 99 86 8a 3d ba 0c -|
    //    53 2e a3 75 a0 f3 11 3d-e7 65 4b ae 3c 42 70 11 -|
    //    dc ca 83 c0 be 3e 97 71-84 69 d6 a8 27 33 9b 3e -|
    //    17 3c a0 4c 64 ca 20 37-a4 11 a9 28 8f b7 18 96 -|
    //    69 15 0d 74 04 75 2a 00-c7 a6 6a be ac b3 f2 fb -|
    //    06 1b 6c 11 bd 96 e2 34-74 5d f5 98 8f 3a 8d 69 -|
    //    08 6f 53 12 4e 39 80 90-ce 8b 5e 88 23 2d fd 55 -|
    //    fd 58 3d 39 27 b3 7c 57-fe 3b ab 62 26 60 e2 d0 -|
    //    c8 f4 02 23 16 c3 52 5d-9f 05 49 a2 71 2d 6d 5b -|
    //    90 dd bf e5 a9 2e f1 85-8a 8a b8 a9 6b 13 cc 8d -|
    //    4c 22 41 ad 32 1e 3b 4b-89 37 66 df 1e a5 4a 03 -|
    //    52 1c d9 19 79 22 d4 a7-3b 47 93 a9 0c 03 6a d8 -|
    //    5f fc c0 75 33 e5 26 da-f7 4a 77 d8 f1 30 80 39 -|
    //    38 1e 86 1d 97 00 9c 0e-ba 00 54 8a c0 12 32 6f -|
    //    3d c4 15 f9 50 f8 ce 95-30 82 04 71 30 82 03 5d -|
    //    a0 03 02 01 02 02 05 03-00 00 00 0f 30 09 06 05 -|
    //    2b 0e 03 02 1d 05 00 30-32 31 30 30 13 06 03 55 -|
    //    04 03 1e 0c 00 52 00 4f-00 44 00 45 00 4e 00 54 -|
    //    30 19 06 03 55 04 07 1e-12 00 57 00 4f 00 52 00 -|
    //    4b 00 47 00 52 00 4f 00-55 00 50 30 1e 17 0d 30 -|
    //    37 30 36 32 30 31 34 35-31 33 35 5a 17 0d 30 37 -|
    //    30 39 31 38 31 34 35 31-33 35 5a 30 7f 31 7d 30 -|
    //    13 06 03 55 04 03 1e 0c-00 52 00 4f 00 44 00 45 -|
    //    00 4e 00 54 30 21 06 03-55 04 07 1e 1a 00 41 00 -|
    //    64 00 6d 00 69 00 6e 00-69 00 73 00 74 00 72 00 -|
    //    61 00 74 00 6f 00 72 30-43 06 03 55 04 05 1e 3c -|
    //    00 31 00 42 00 63 00 4b-00 65 00 64 00 79 00 32 -|
    //    00 6b 00 72 00 4f 00 34-00 2f 00 4d 00 43 00 44 -|
    //    00 4c 00 49 00 31 00 41-00 48 00 5a 00 63 00 50 -|
    //    00 69 00 61 00 73 00 3d-00 0d 00 0a 30 82 01 22 -|
    //    30 0d 06 09 2a 86 48 86-f7 0d 01 01 01 05 00 03 -|
    //    82 01 0f 00 30 82 01 0a-02 82 01 01 00 88 ad 7c -|
    //    8f 8b 82 76 5a bd 8f 6f-62 18 e1 d9 aa 41 fd ed -|
    //    68 01 c6 34 35 b0 29 04-ca 4a 4a 1c 7e 80 14 f7 -|
    //    8e 77 b8 25 ff 16 47 6f-bd e2 34 3d 2e 02 b9 53 -|
    //    e4 33 75 ad 73 28 80 a0-4d fc 6c c0 22 53 1b 2c -|
    //    f8 f5 01 60 19 7e 79 19-39 8d b5 ce 39 58 dd 55 -|
    //    24 3b 55 7b 43 c1 7f 14-2f b0 64 3a 54 95 2b 88 -|
    //    49 0c 61 2d ac f8 45 f5-da 88 18 5f ae 42 f8 75 -|
    //    c7 26 6d b5 bb 39 6f cc-55 1b 32 11 38 8d e4 e9 -|
    //    44 84 11 36 a2 61 76 aa-4c b4 e3 55 0f e4 77 8e -|
    //    de e3 a9 ea b7 41 94 00-58 aa c9 34 a2 98 c6 01 -|
    //    1a 76 14 01 a8 dc 30 7c-77 5a 20 71 5a a2 3f af -|
    //    13 7e e8 fd 84 a2 5b cf-25 e9 c7 8f a8 f2 8b 84 -|
    //    c7 04 5e 53 73 4e 0e 89-a3 3c e7 68 5c 24 b7 80 -|
    //    53 3c 54 c8 c1 53 aa 71-71 3d 36 15 d6 6a 9d 7d -|
    //    de ae f9 e6 af 57 ae b9-01 96 5d e0 4d cd ed c8 -|
    //    d7 f3 01 03 38 10 be 7c-42 67 01 a7 23 02 03 01 -|
    //    00 01 a3 82 01 47 30 82-01 43 30 14 06 09 2b 06 -|
    //    01 04 01 82 37 12 04 01-01 ff 04 04 01 00 05 00 -|
    //    30 3c 06 09 2b 06 01 04-01 82 37 12 02 01 01 ff -|
    //    04 2c 4d 00 69 00 63 00-72 00 6f 00 73 00 6f 00 -|
    //    66 00 74 00 20 00 43 00-6f 00 72 00 70 00 6f 00 -|
    //    72 00 61 00 74 00 69 00-6f 00 6e 00 00 00 30 56 -|
    //    06 09 2b 06 01 04 01 82-37 12 05 01 01 ff 04 46 -|
    //    00 30 00 00 01 00 00 00-ff 00 00 00 00 04 00 00 -|
    //    1c 00 08 00 24 00 16 00-3a 00 01 00 41 00 30 00 -|
    //    32 00 00 00 41 00 30 00-32 00 2d 00 36 00 2e 00 -|
    //    30 00 30 00 2d 00 53 00-00 00 06 00 00 00 00 80 -|
    //    64 80 00 00 00 00 30 6e-06 09 2b 06 01 04 01 82 -|
    //    37 12 06 01 01 ff 04 5e-00 30 00 00 00 00 0e 00 -|
    //    3e 00 52 00 4f 00 44 00-45 00 4e 00 54 00 00 00 -|
    //    37 00 38 00 34 00 34 00-30 00 2d 00 30 00 30 00 -|
    //    36 00 2d 00 35 00 38 00-36 00 37 00 30 00 34 00 -|
    //    35 00 2d 00 37 00 30 00-33 00 34 00 37 00 00 00 -|
    //    57 00 4f 00 52 00 4b 00-47 00 52 00 4f 00 55 00 -|
    //    50 00 00 00 00 00 30 25-06 03 55 1d 23 01 01 ff -|
    //    04 1b 30 19 a1 10 a4 0e-52 00 4f 00 44 00 45 00 -|
    //    4e 00 54 00 00 00 82 05-03 00 00 00 0f 30 09 06 -|
    //    05 2b 0e 03 02 1d 05 00-03 82 01 01 00 13 1b dc -|
    //    89 d2 fc 54 0c ee 82 45-68 6a 72 c3 3e 17 73 96 -|
    //    53 44 39 50 0e 0b 9f 95-d6 2c 6b 53 14 9c e5 55 -|
    //    ed 65 df 2a eb 5c 64 85-70 1f bc 96 cf a3 76 b1 -|
    //    72 3b e1 f6 ad ad ad 2a-14 af ba d0 d6 d5 6d 55 -|
    //    ec 1e c3 4b ba 06 9c 59-78 93 64 87 4b 03 f9 ee -|
    //    4c dd 36 5b bd d4 e5 4c-4e da 7b c1 ae 23 28 9e -|
    //    77 6f 0f e6 94 fe 05 22-00 ab 63 5b e1 82 45 a6 -|
    //    ec 1f 6f 2c 7b 56 de 78-25 7d 10 60 0e 53 42 4b -|
    //    6c 7a 6b 5d c9 d5 a6 ae-c8 c8 52 29 d6 42 56 02 -|
    //    ec f9 23 a8 8c 8d 89 c9-7c 84 07 fc 33 e1 1e ea -|
    //    e2 8f 2b be 8f a9 d3 d1-e1 5e 0b dc b6 43 6e 33 -|
    //    0a f4 2e 9d 0c c9 58 54-34 aa e1 d2 a2 e4 90 02 -|
    //    23 26 a0 92 26 26 0a 83-b4 4d d9 4b ef eb 9d a9 -|
    //    24 3f 92 8b db 04 7b 9d-64 91 a4 4b d2 6e 51 05 -|
    //    08 c9 91 af 31 26 55 21-b1 ea ce a3 a4 0d 5e 4c -|
    //    46 db 16 2d 98 dc 60 19-b8 1b b9 cd fb 31 00    -/ 
    //    LicenseInfo::pbLicenseInfo
    //    0x7f7: MACData (0x10 bytes)
    //    ed-e8 bf d6 13 a0 f5 80 -\|
    //    4a e5 ff 85 16 fa cb 1f -/ MACData

//    //    The decrypted LicenseInfo for the above data is 
//    //  , 0x00, 0x00, 0x06, 0x00
//    //  , 0x0e, 0x00, 0x00, 0x00
//    //  , 0x6d, 0x69, 0x63, 0x72, 0x6f, 0x73, 0x6f, 0x66, 0x74, 0x2e, 0x63, 0x6f, 0x6d, 0x00
//    //  , 0x2c, 0x00, 0x00, 0x00
//    //  , 0x4d, 0x00, 0x69, 0x00, 0x63, 0x00, 0x72, 0x00
//    //  , 0x6f, 0x00, 0x73, 0x00, 0x6f, 0x00, 0x66, 0x00
//    //  , 0x74, 0x00, 0x20, 0x00, 0x43, 0x00, 0x6f, 0x00
//    //  , 0x72, 0x00, 0x70, 0x00, 0x6f, 0x00, 0x72, 0x00
//    //  , 0x61, 0x00, 0x74, 0x00, 0x69, 0x00, 0x6f, 0x00
//    //  , 0x6e, 0x00, 0x00, 0x00
//    //     
//    //    08 -\|
//    //    00 -|
//    //    00 -|
//    //    00 -/ LicenseInfo::cbProductId

//    //    41 00 30 00 32 00 00 00 ->LicenseInfo::pbProductId
//    //     
//    //    99 -\|
//    //    07 -|
//    //    00 -|
//    //    00 -/ LicenseInfo::cbLicenseInfo

//    //                      30 82 07 95 06 09 2a 86 48 86 -\|
//    //    f7 0d 01 07 02 a0 82 07-86 30 82 07 82 02 01 01 -|
//    //    31 00 30 0b 06 09 2a 86-48 86 f7 0d 01 07 01 a0 -|
//    //    82 07 6a 30 82 02 f1 30-82 01 dd a0 03 02 01 02 -|
//    //    02 08 01 9e 27 4d 68 ac-ed 20 30 09 06 05 2b 0e -|
//    //    03 02 1d 05 00 30 32 31-30 30 13 06 03 55 04 03 -|
//    //    1e 0c 00 52 00 4f 00 44-00 45 00 4e 00 54 30 19 -|
//    //    06 03 55 04 07 1e 12 00-57 00 4f 00 52 00 4b 00 -|
//    //    47 00 52 00 4f 00 55 00-50 30 1e 17 0d 37 30 30 -|
//    //    35 33 30 31 30 33 36 31-38 5a 17 0d 34 39 30 35 -|
//    //    33 30 31 30 33 36 31 38-5a 30 32 31 30 30 13 06 -|
//    //    03 55 04 03 1e 0c 00 52-00 4f 00 44 00 45 00 4e -|
//    //    00 54 30 19 06 03 55 04-07 1e 12 00 57 00 4f 00 -|
//    //    52 00 4b 00 47 00 52 00-4f 00 55 00 50 30 82 01 -|
//    //    22 30 0d 06 09 2a 86 48-86 f7 0d 01 01 01 05 00 -|
//    //    03 82 01 0f 00 30 82 01-0a 02 82 01 01 00 88 ad -|
//    //    7c 8f 8b 82 76 5a bd 8f-6f 62 18 e1 d9 aa 41 fd -|
//    //    ed 68 01 c6 34 35 b0 29-04 ca 4a 4a 1c 7e 80 14 -|
//    //    f7 8e 77 b8 25 ff 16 47-6f bd e2 34 3d 2e 02 b9 -|
//    //    53 e4 33 75 ad 73 28 80-a0 4d fc 6c c0 22 53 1b -|
//    //    2c f8 f5 01 60 19 7e 79-19 39 8d b5 ce 39 58 dd -|
//    //    55 24 3b 55 7b 43 c1 7f-14 2f b0 64 3a 54 95 2b -|
//    //    88 49 0c 61 2d ac f8 45-f5 da 88 18 5f ae 42 f8 -|
//    //    75 c7 26 6d b5 bb 39 6f-cc 55 1b 32 11 38 8d e4 -|
//    //    e9 44 84 11 36 a2 61 76-aa 4c b4 e3 55 0f e4 77 -|
//    //    8e de e3 a9 ea b7 41 94-00 58 aa c9 34 a2 98 c6 -|
//    //    01 1a 76 14 01 a8 dc 30-7c 77 5a 20 71 5a a2 3f -|
//    //    af 13 7e e8 fd 84 a2 5b-cf 25 e9 c7 8f a8 f2 8b -|
//    //    84 c7 04 5e 53 73 4e 0e-89 a3 3c e7 68 5c 24 b7 -|
//    //    80 53 3c 54 c8 c1 53 aa-71 71 3d 36 15 d6 6a 9d -|
//    //    7d de ae f9 e6 af 57 ae-b9 01 96 5d e0 4d cd ed -|
//    //    c8 d7 f3 01 03 38 10 be-7c 42 67 01 a7 23 02 03 -|
//    //    01 00 01 a3 13 30 11 30-0f 06 03 55 1d 13 04 08 -|
//    //    30 06 01 01 ff 02 01 00-30 09 06 05 2b 0e 03 02 -|
//    //    1d 05 00 03 82 01 01 00-70 db 21 2b 84 9a 7a c3 -|
//    //    b1 68 fa c0 00 8b 71 ab-43 9f b6 7b b7 1f 20 83 -|
//    //    ac 0a b5 0e ad b6 36 ef-65 17 99 86 8a 3d ba 0c -|
//    //    53 2e a3 75 a0 f3 11 3d-e7 65 4b ae 3c 42 70 11 -|
//    //    dc ca 83 c0 be 3e 97 71-84 69 d6 a8 27 33 9b 3e -|
//    //    17 3c a0 4c 64 ca 20 37-a4 11 a9 28 8f b7 18 96 -|
//    //    69 15 0d 74 04 75 2a 00-c7 a6 6a be ac b3 f2 fb -|
//    //    06 1b 6c 11 bd 96 e2 34-74 5d f5 98 8f 3a 8d 69 -|
//    //    08 6f 53 12 4e 39 80 90-ce 8b 5e 88 23 2d fd 55 -|
//    //    fd 58 3d 39 27 b3 7c 57-fe 3b ab 62 26 60 e2 d0 -|
//    //    c8 f4 02 23 16 c3 52 5d-9f 05 49 a2 71 2d 6d 5b -|
//    //    90 dd bf e5 a9 2e f1 85-8a 8a b8 a9 6b 13 cc 8d -|
//    //    4c 22 41 ad 32 1e 3b 4b-89 37 66 df 1e a5 4a 03 -|
//    //    52 1c d9 19 79 22 d4 a7-3b 47 93 a9 0c 03 6a d8 -|
//    //    5f fc c0 75 33 e5 26 da-f7 4a 77 d8 f1 30 80 39 -|
//    //    38 1e 86 1d 97 00 9c 0e-ba 00 54 8a c0 12 32 6f -|
//    //    3d c4 15 f9 50 f8 ce 95-30 82 04 71 30 82 03 5d -|
//    //    a0 03 02 01 02 02 05 03-00 00 00 0f 30 09 06 05 -|
//    //    2b 0e 03 02 1d 05 00 30-32 31 30 30 13 06 03 55 -|
//    //    04 03 1e 0c 00 52 00 4f-00 44 00 45 00 4e 00 54 -|
//    //    30 19 06 03 55 04 07 1e-12 00 57 00 4f 00 52 00 -|
//    //    4b 00 47 00 52 00 4f 00-55 00 50 30 1e 17 0d 30 -|
//    //    37 30 36 32 30 31 34 35-31 33 35 5a 17 0d 30 37 -|
//    //    30 39 31 38 31 34 35 31-33 35 5a 30 7f 31 7d 30 -|
//    //    13 06 03 55 04 03 1e 0c-00 52 00 4f 00 44 00 45 -|
//    //    00 4e 00 54 30 21 06 03-55 04 07 1e 1a 00 41 00 -|
//    //    64 00 6d 00 69 00 6e 00-69 00 73 00 74 00 72 00 -|
//    //    61 00 74 00 6f 00 72 30-43 06 03 55 04 05 1e 3c -|
//    //    00 31 00 42 00 63 00 4b-00 65 00 64 00 79 00 32 -|
//    //    00 6b 00 72 00 4f 00 34-00 2f 00 4d 00 43 00 44 -|
//    //    00 4c 00 49 00 31 00 41-00 48 00 5a 00 63 00 50 -|
//    //    00 69 00 61 00 73 00 3d-00 0d 00 0a 30 82 01 22 -|
//    //    30 0d 06 09 2a 86 48 86-f7 0d 01 01 01 05 00 03 -|
//    //    82 01 0f 00 30 82 01 0a-02 82 01 01 00 88 ad 7c -|
//    //    8f 8b 82 76 5a bd 8f 6f-62 18 e1 d9 aa 41 fd ed -|
//    //    68 01 c6 34 35 b0 29 04-ca 4a 4a 1c 7e 80 14 f7 -|
//    //    8e 77 b8 25 ff 16 47 6f-bd e2 34 3d 2e 02 b9 53 -|
//    //    e4 33 75 ad 73 28 80 a0-4d fc 6c c0 22 53 1b 2c -|
//    //    f8 f5 01 60 19 7e 79 19-39 8d b5 ce 39 58 dd 55 -|
//    //    24 3b 55 7b 43 c1 7f 14-2f b0 64 3a 54 95 2b 88 -|
//    //    49 0c 61 2d ac f8 45 f5-da 88 18 5f ae 42 f8 75 -|
//    //    c7 26 6d b5 bb 39 6f cc-55 1b 32 11 38 8d e4 e9 -|
//    //    44 84 11 36 a2 61 76 aa-4c b4 e3 55 0f e4 77 8e -|
//    //    de e3 a9 ea b7 41 94 00-58 aa c9 34 a2 98 c6 01 -|
//    //    1a 76 14 01 a8 dc 30 7c-77 5a 20 71 5a a2 3f af -|
//    //    13 7e e8 fd 84 a2 5b cf-25 e9 c7 8f a8 f2 8b 84 -|
//    //    c7 04 5e 53 73 4e 0e 89-a3 3c e7 68 5c 24 b7 80 -|
//    //    53 3c 54 c8 c1 53 aa 71-71 3d 36 15 d6 6a 9d 7d -|
//    //    de ae f9 e6 af 57 ae b9-01 96 5d e0 4d cd ed c8 -|
//    //    d7 f3 01 03 38 10 be 7c-42 67 01 a7 23 02 03 01 -|
//    //    00 01 a3 82 01 47 30 82-01 43 30 14 06 09 2b 06 -|
//    //    01 04 01 82 37 12 04 01-01 ff 04 04 01 00 05 00 -|
//    //    30 3c 06 09 2b 06 01 04-01 82 37 12 02 01 01 ff -|
//    //    04 2c 4d 00 69 00 63 00-72 00 6f 00 73 00 6f 00 -|
//    //    66 00 74 00 20 00 43 00-6f 00 72 00 70 00 6f 00 -|
//    //    72 00 61 00 74 00 69 00-6f 00 6e 00 00 00 30 56 -|
//    //    06 09 2b 06 01 04 01 82-37 12 05 01 01 ff 04 46 -|
//    //    00 30 00 00 01 00 00 00-ff 00 00 00 00 04 00 00 -|
//    //    1c 00 08 00 24 00 16 00-3a 00 01 00 41 00 30 00 -|
//    //    32 00 00 00 41 00 30 00-32 00 2d 00 36 00 2e 00 -|
//    //    30 00 30 00 2d 00 53 00-00 00 06 00 00 00 00 80 -|
//    //    64 80 00 00 00 00 30 6e-06 09 2b 06 01 04 01 82 -|
//    //    37 12 06 01 01 ff 04 5e-00 30 00 00 00 00 0e 00 -|
//    //    3e 00 52 00 4f 00 44 00-45 00 4e 00 54 00 00 00 -|
//    //    37 00 38 00 34 00 34 00-30 00 2d 00 30 00 30 00 -|
//    //    36 00 2d 00 35 00 38 00-36 00 37 00 30 00 34 00 -|
//    //    35 00 2d 00 37 00 30 00-33 00 34 00 37 00 00 00 -|
//    //    57 00 4f 00 52 00 4b 00-47 00 52 00 4f 00 55 00 -|
//    //    50 00 00 00 00 00 30 25-06 03 55 1d 23 01 01 ff -|
//    //    04 1b 30 19 a1 10 a4 0e-52 00 4f 00 44 00 45 00 -|
//    //    4e 00 54 00 00 00 82 05-03 00 00 00 0f 30 09 06 -|
//    //    05 2b 0e 03 02 1d 05 00-03 82 01 01 00 13 1b dc -|
//    //    89 d2 fc 54 0c ee 82 45-68 6a 72 c3 3e 17 73 96 -|
//    //    53 44 39 50 0e 0b 9f 95-d6 2c 6b 53 14 9c e5 55 -|
//    //    ed 65 df 2a eb 5c 64 85-70 1f bc 96 cf a3 76 b1 -|
//    //    72 3b e1 f6 ad ad ad 2a-14 af ba d0 d6 d5 6d 55 -|
//    //    ec 1e c3 4b ba 06 9c 59-78 93 64 87 4b 03 f9 ee -|
//    //    4c dd 36 5b bd d4 e5 4c-4e da 7b c1 ae 23 28 9e -|
//    //    77 6f 0f e6 94 fe 05 22-00 ab 63 5b e1 82 45 a6 -|
//    //    ec 1f 6f 2c 7b 56 de 78-25 7d 10 60 0e 53 42 4b -|
//    //    6c 7a 6b 5d c9 d5 a6 ae-c8 c8 52 29 d6 42 56 02 -|
//    //    ec f9 23 a8 8c 8d 89 c9-7c 84 07 fc 33 e1 1e ea -|
//    //    e2 8f 2b be 8f a9 d3 d1-e1 5e 0b dc b6 43 6e 33 -|
//    //    0a f4 2e 9d 0c c9 58 54-34 aa e1 d2 a2 e4 90 02 -|
//    //    23 26 a0 92 26 26 0a 83-b4 4d d9 4b ef eb 9d a9 -|
//    //    24 3f 92 8b db 04 7b 9d-64 91 a4 4b d2 6e 51 05 -|
//    //    08 c9 91 af 31 26 55 21-b1 ea ce a3 a4 0d 5e 4c -|
//    //    46 db 16 2d 98 dc 60 19-b8 1b b9 cd fb 31 00    -/ 
//    //    LicenseInfo::pbLicenseInfo
//    //    0x7f7: MACData (0x10 bytes)
//    //    ed-e8 bf d6 13 a0 f5 80 -\|
//    //    4a e5 ff 85 16 fa cb 1f -/ MACData


    struct NewLicense_Recv
    {
        uint8_t wMsgType;
        uint8_t bVersion;
        uint16_t wMsgSize;

        struct LicenseInfo {
            uint16_t wBlobType;
            uint16_t wBlobLen;

            // Following Data in encrypted using licence key

            //    LicenseInfo::dwVersion (4 bytes): The content and format of this field are the same 
            // as the dwVersion field of the Product Information (section 2.2.2.1.1) structure.
            // it's a 32-bit unsigned integer that contains the license version information. The high-order word
            // contains the major version of the operating system on which the terminal server is running, while
            // the low-order word contains the minor version.<6>
            uint32_t dwVersion;

            //     LicenseInfo::cbScope (4 bytes): A 32-bit unsigned integer that contains the number of 
            // bytes in the string contained in the pbScope field.
            uint32_t cbScope;

            //     LicenseInfo::pbScope (variable): Contains the NULL-terminated ANSI character set string
            // giving the name of the issuer of this license. For example, for licenses issued
            // by TailSpin Toys, this field contains the string "TailSpin Toys".
            uint8_t pbScope[128]; // check the actual size is not too large for what we provide

            //     LicenseInfo::cbCompanyName (4 bytes): The content and format of this field are the same as
            // the cbCompanyName field of the Product Information structure. An unsigned 32-bit integer that contains
            // the number of bytes in the pbCompanyName field, including the terminating null character. 
            // This value MUST be greater than zero.
            uint32_t cbCompanyName;

            //     LicenseInfo::pbCompanyName (variable): The content and format of this field are the same as
            // the pbCompanyName field of the Product Information structure. Contains a null-terminated Unicode string
            // that specifies the company name.<7> 
            uint8_t pbCompanyName[128]; // check the actual size is not too large for what we provide

            //     LicenseInfo::cbProductId (4 bytes): The content and format of this field are the same as 
            // the cbProductId field of the Product Information structure. An unsigned 32-bit integer that contains 
            // the number of bytes in the pbProductId field, including the terminating null character. This value MUST
            // be greater than zero.
            uint32_t cbProductId;

            //     LicenseInfo::pbProductId (variable): The content and format of this field are the same as
            // the pbProductId field of the Product Information structure. Contains a null-terminated Unicode string 
            // that identifies the type of the license that is required by the terminal server. 
            // It MAY have the following string value. "A02" Per device or per user license
            uint8_t pbProductId[128]; // check the actual size is not too large for what we provide

            //     LicenseInfo::cbLicenseInfo (4 bytes): A 32-bit unsigned integer that contains the number
            // of bytes of binary data in the pbLicenseInfo field.
            uint32_t cbLicenseInfo;

            //     LicenseInfo::pbLicenseInfo (variable): This field contains the CAL issued to the client by
            // the license server. This license consists of an X.509 certificate chain generated 
            // by the license server. The binary data contained in this field is opaque to the 
            // client. The client sends this information back to the server in the Client License 
            // Information message.
            uint8_t pbLicenseInfo[65535];
        } licenseInfo;

        uint8_t MACData[16];

        NewLicense_Recv(Stream & stream, uint8_t license_key[]){
            this->wMsgType = stream.in_uint8();
            this->bVersion = stream.in_uint8();
            this->wMsgSize = stream.in_uint16_le();

            this->licenseInfo.wBlobType = stream.in_uint16_le();
            this->licenseInfo.wBlobLen = stream.in_uint16_le();

            // following data is encrypted using license_key
            RC4_KEY crypt_key;
            RC4_set_key(&crypt_key, 16, license_key);
            RC4(&crypt_key, this->licenseInfo.wBlobLen, stream.p, stream.p);

            // now it's unencrypted, we can read it
            this->licenseInfo.dwVersion = stream.in_uint32_le();
            this->licenseInfo.cbScope = stream.in_uint32_le();


//            stream.in_copy_bytes(this->LicenseInfo.pbScope, this->LicenseInfo.cbScope);
//            this->LicenseInfo.cbCompanyName = stream.in_uint32_le();
//            stream.in_copy_bytes(this->LicenseInfo.pbCompanyName, this->LicenseInfo.cbCompanyName);
//            this->LicenseInfo.cbProductId = stream.in_uint32_le();
//            stream.in_copy_bytes(this->LicenseInfo.pbProductId, this->LicenseInfo.cbProductId);
//            this->LicenseInfo.cbLicenseInfo = stream.in_uint32_le();
//            stream.in_copy_bytes(this->LicenseInfo.pbLicenseInfo, this->LicenseInfo.cbLicenseInfo);

//            stream.in_copy_bytes(this->MACData, LICENSE_SIGNATURE_SIZE);

            stream.end = stream.p;
            if (stream.p != stream.end){
                LOG(LOG_ERR, "PlatformChallenge_Recv : unparsed data %d", stream.end - stream.p);
                throw Error(ERR_LIC);
            }

//                        stream.in_skip_bytes(2); /* 3d 45 - unknown */
//                        int len1 = stream.in_uint16_le();
//                        RC4_KEY crypt_key;
//                        RC4_set_key(&crypt_key, 16, this->lic_layer_license_key);
//                        RC4(&crypt_key, len1, stream.p, stream.p);
//                        int check = stream.in_uint16_le();
//                        license_issued = 1;

//                        stream.in_skip_bytes(2); /* pad */

//                        uint32_t len2 = stream.in_uint32_le();
//                        stream.in_skip_bytes(len2);

//                        uint32_t len3 = stream.in_uint32_le();
//                        stream.in_skip_bytes(len3);

//                        uint32_t len4 = stream.in_uint32_le();
//                        stream.in_skip_bytes(len4);

//                        uint32_t len5 = stream.in_uint32_le();


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
