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


#pragma once

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cinttypes>


#include "utils/log.hpp"
#include "utils/crypto/ssl_lib.hpp"
#include "utils/stream.hpp"
#include "utils/hexdump.hpp"
#include "core/error.hpp"
#include "system/ssl_rc4.hpp"


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

// Client                                  Server
//   | ------------LICENSE_INFO------------->|

// Client                                  Server
//   | <-----------LICENSE_REQUEST ----------|
//   | ------------NEW_LICENSE_REQUEST ----->|
//   | <----ERROR_ALERT:VALID_CLIENT_DATA----|

// Client                                  Server
//   | <-----------LICENSE_REQUEST ----------|
//   | ------------LICENSE_INFO------------->|

// Client                                  Server
//   | <-----------NEW_LICENSE---------------|
//   | --------------------------------------|

// Client                                  Server
//   | <-----------ERROR_ALERT---------------|

// Client                                  Server
//   | ------------ERROR_ALERT-------------->|

// Client                                  Server
//   | <-----------UPGRADE_LICENSE-----------|
//   | --------------------------------------|

// Client                                  Server
//   | <-----------PLATFORM_CHALLENGE--------|
//   | -------PLATFORM_CHALLENGE_RESPONSE--->|

namespace LIC
{
    enum {
        LICENSE_TOKEN_SIZE             = 10,
        LICENSE_HWID_SIZE              = 20,
        LICENSE_SIGNATURE_SIZE         = 16
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
        LICENSE_TAG_HOST            = 0x0010
    };

    enum {
        // sent by client
        ERR_INVALID_SERVER_CERTIFICATE   = 0x00000001,
        ERR_NO_LICENSE                   = 0x00000002,

        // Sent by server:
        ERR_INVALID_SCOPE                = 0x00000004,
        ERR_NO_LICENSE_SERVER            = 0x00000006,
        STATUS_VALID_CLIENT              = 0x00000007,
        ERR_INVALID_CLIENT               = 0x00000008,
        ERR_INVALID_PRODUCTID            = 0x0000000B,
        ERR_INVALID_MESSAGE_LEN          = 0x0000000C,

        //    Sent by client and server:
        ERR_INVALID_MAC                  = 0x00000003
    };

    enum {
        ST_TOTAL_ABORT            = 0x00000001,
        ST_NO_TRANSITION          = 0x00000002,
        ST_RESET_PHASE_TO_START   = 0x00000003,
        ST_RESEND_LAST_MESSAGE    = 0x00000004
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
        BB_CLIENT_MACHINE_NAME_BLOB = 0x0010
    };

    enum {
        KEY_EXCHANGE_ALG_RSA        = 0x01
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

        explicit RecvFactory(InStream & stream)
        {
            if (!stream.in_check_rem(4)){
                LOG(LOG_ERR, "Not enough data to read licence info header, need %u, got %zu", 4u, stream.in_remain());
                throw Error(ERR_LIC);
            }
            this->tag = stream.in_uint8();
            this->flags = stream.in_uint8();
            this->wMsgSize = stream.in_uint16_le();
            if (this->wMsgSize > stream.get_capacity()){
                LOG(LOG_ERR, "Not enough data to read licence data, need %" PRIu16 ", got %zu", this->wMsgSize, stream.get_capacity());
                throw Error(ERR_LIC);
            }
            // TODO Factory does not read anything, write it using a cleaner method
//            stream.p -= 4;
            stream.rewind();
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

        explicit LicenseRequest_Recv(InStream & stream){
            unsigned expected = 4 + SEC_RANDOM_SIZE; /* tag(1) + flags(1) + wMsgSize(2) + server_random(SEC_RANDOM_SIZE) */
            if (!stream.in_check_rem(expected)){
                LOG(LOG_ERR, "Truncated License Request_Recv: need %u remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_LIC);
            }

            this->tag = stream.in_uint8();
            this->flags = stream.in_uint8();
            this->wMsgSize = stream.in_uint16_le();

            stream.in_copy_bytes(this->server_random, SEC_RANDOM_SIZE);

            // TODO Add missing productInfo field
            stream.in_skip_bytes(stream.in_remain());

            // TODO Never true
            if (stream.in_remain()){
                LOG(LOG_ERR, "License Request_Recv: unparsed data %zu", stream.in_remain());
                throw Error(ERR_LIC);
            }
        }
    };

    //    2.2.2.2 Client New License Request (CLIENT_NEW_LICENSE_REQUEST)
    //    ---------------------------------------------------------------

    //    The Client New License Request packet is sent to a server when the
    // client cannot find a license matching the product information provided
    // in the Server License Request message. This message is interpreted as
    // a new license request by the server, and the server SHOULD attempt to
    // issue a new license to the client on receipt of this message.



    //   PreferredKeyExchangeAlg (4 bytes): A 32-bit unsigned integer that
    // indicates the key exchange algorithm chosen by the client. It MUST be
    // set to KEY_EXCHANGE_ALG_RSA (0x00000001), which indicates an RSA-based
    // key exchange with a 512-bit asymmetric key.<9>

    //   PlatformId (4 bytes): A 32-bit unsigned integer. This field is composed
    // of two identifiers: the operating system identifier and the independent
    // software vendor (ISV) identifier. The platform ID is composed of the
    // logical OR of these two values.

    // - The most significant byte of the PlatformId field contains the operating
    // system version of the client.<10>

    // - The second most significant byte of the PlatformId field identifies the
    // ISV that provided the client image.<11>

    // - The remaining two bytes in the PlatformId field are used by the ISV to
    // identify the build number of the operating system.<12>

    //    ClientRandom (32 bytes): A 32-byte random number generated by the client
    // using a cryptographically secure pseudo-random number generator. The
    // ClientRandom and ServerRandom (see section 2.2.2.1) values, along with the
    // data in the EncryptedPreMasterSecret field, are used to generate licensing
    // encryption keys (see section 5.1.3). These keys are used to encrypt licensing
    // protocol messages (see sections 5.1.4 and 5.1.5).

    //    EncryptedPreMasterSecret (variable): A Licensing Binary BLOB structure
    // (see [MS-RDPBCGR] section 2.2.1.12.1.2) of type BB_RANDOM_BLOB (0x0002). This
    // BLOB contains an encrypted 48-byte random number. For instructions on how to
    // encrypt this random number, see section 5.1.2.1.

    //    ClientUserName (variable): A Licensing Binary BLOB structure (see [MS-RDPBCGR]
    // section 2.2.1.12.1.2) of type BB_CLIENT_USER_NAME_BLOB (0x000F). This BLOB contains
    // the client user name string in null-terminated ANSI character set format and is
    // used along with the ClientMachineName BLOB to keep track of licenses issued to
    // clients.

    //    ClientMachineName (variable): A Licensing Binary BLOB structure (see [MS-RDPBCGR]
    // section 2.2.1.12.1.2) of type BB_CLIENT_MACHINE_NAME_BLOB (0x0010). This BLOB
    // contains the client machine name string in null-terminated ANSI character set format
    // and is used along with the ClientUserName BLOB to keep track of licenses issued
    // to clients.


    //    4.2 CLIENT NEW LICENSE REQUEST
    //    ------------------------------
    //    This topic has not yet been rated - Rate this topic

    //    If the client does not already have a license in its store, it sends the Client New License Request message as a response to the Server License Request message. See sections 2.2.2.2 and 3.3.5.2. For information on ExtendedError, see [MS-EERR].

    //    00000000:  13 83 55 01 01 00 00 00-00 00 01 04 dc 73 a0 c8  ..U..........s..
    //    00000010:  69 25 6b 18 af 0b 94 7a-a9 a5 20 af 8b bc 0d cc  i%k....z.. .....
    //    00000020:  a3 95 b7 b9 eb 81 5d be-0a 10 9c d8 02 00 08 01  ......].........
    //    00000030:  da 9c 5d a6 68 9d a3 90-67 24 f3 3a ea a1 e2 68  ..].h...g$.:...h
    //    00000040:  ad 12 f5 f6 0b 7a ac 92-b1 69 6f 42 55 8a a0 e2  .....z...ioBU...
    //    00000050:  9b 2c d0 c7 ee 33 6c 47-79 c3 1e bf 03 8b 95 70  .,...3lGy......p
    //    00000060:  07 a2 be ee 54 02 68 f8-90 d7 fe 2c 08 e1 6b 2d  ....T.h....,..k-
    //    00000070:  ff 94 76 72 5f 7a 76 75-32 55 cc 58 61 63 a5 64  ..vr_zvu2U.Xac.d
    //    00000080:  f1 6e c3 07 81 82 6f 88-73 62 fc 28 65 91 c2 c8  .n....o.sb.(e...
    //    00000090:  9f 05 b0 d3 93 12 bf 6a-50 18 99 2d 4d c4 7f 74  .......jP..-M..t
    //    000000A0:  d3 30 9f 16 78 a5 df aa-83 65 4f 77 30 42 e0 d7  .0..x....eOw0B..
    //    000000B0:  69 c8 4d a5 73 11 59 35-b9 a7 e2 b0 f6 e3 b9 39  i.M.s.Y5.......9
    //    000000C0:  c3 d4 e4 6b ca 40 9a ac-66 e6 1a a4 1b 39 7e 09  ...k.@..f....9~.
    //    000000D0:  e3 72 99 dd 90 62 55 97-a9 04 c7 51 aa a2 01 cb  .r...bU....Q....
    //    000000E0:  5a 63 4d 1a e5 99 c3 b1-2a 73 e8 9a 00 46 92 59  ZcM.....*s...F.Y
    //    000000F0:  39 a3 80 a1 ac 90 52 ea-63 81 49 7d f3 2d 5c c3  9.....R.c.I}.-\.
    //    00000100:  19 9f ed fe 81 1d 8c 04-1c d9 23 d2 6d 80 84 f3  ..........#.m...
    //    00000110:  00 f2 b1 69 2f cd b3 9f-69 ee 60 3e 4b b5 be 5a  ...i/...i.`>K..Z
    //    00000120:  09 83 0b bc 3d 3e 05 47-65 96 31 8c 6b c5 e6 a0  ....=>.Ge.1.k...
    //    00000130:  00 00 00 00 00 00 00 00-0f 00 0e 00 41 64 6d 69  ............Admi
    //    00000140:  6e 69 73 74 72 61 74 6f-72 00 10 00 07 00 52 4f  nistrator.....RO
    //    00000150:  44 45 4e 54 00                                   DENT.

    //    0x00: LICENSE_PREAMBLE (4 bytes)
    //    13 -> LICENSE_PREAMBLE::bMsgType = NEW_LICENSE_REQUEST
    //
    //    83 -> LICENSE_PREAMBLE::bVersion = 0x80
    //               (ExtendedError supported) | 0x3 (RDP 5.0,5.2,6.0)
    //
    //    55 -\|
    //    01 -/ LICENSE_PREAMBLE::wMsgSize = 0x155
    //    0x04: PreferredKeyGenAlgo (4 bytes)
    //    01 -\|
    //    00 -|
    //    00 -|
    //    00 -/ PreferredKeyGenAlgo = 0x01
    //    0x08: PlatformId (4 bytes)
    //    00 -\|
    //    00 -|
    //    01 -|
    //    04 -/ PlatformId = CLIENT_OS_ID_WINNT_POST_52 |
    //                         CLIENT__IMAGE_ID_MICROSOFT = 0x04010000
    //    0x0c: ClientRandom (0x20 bytes)
    //    dc 73 a0 c8 69 25 6b 18
    //    af 0b 94 7a a9 a5 20 af
    //    8b bc 0d cc a3 95 b7 b9
    //    eb 81 5d be 0a 10 9c d8
    //    0x2c: EncryptedPreMaster (2 + 2 + 0x108 = 0x10c bytes)
    //    02 -\|
    //    00 -/ EncryptedPreMaster::wBlobType
    //
    //    08 -\|
    //    01 -/ EncryptedPreMaster::wBlobLen = 0x108 bytes

    //    da 9c 5d a6 68 9d a3 90-67 24 f3 3a ea a1 e2 68 -\|
    //    ad 12 f5 f6 0b 7a ac 92-b1 69 6f 42 55 8a a0 e2 -|
    //    9b 2c d0 c7 ee 33 6c 47-79 c3 1e bf 03 8b 95 70 -|
    //    07 a2 be ee 54 02 68 f8-90 d7 fe 2c 08 e1 6b 2d -|
    //    ff 94 76 72 5f 7a 76 75-32 55 cc 58 61 63 a5 64 -|
    //    f1 6e c3 07 81 82 6f 88-73 62 fc 28 65 91 c2 c8 -|
    //    9f 05 b0 d3 93 12 bf 6a-50 18 99 2d 4d c4 7f 74 -|
    //    d3 30 9f 16 78 a5 df aa-83 65 4f 77 30 42 e0 d7 -|
    //    69 c8 4d a5 73 11 59 35-b9 a7 e2 b0 f6 e3 b9 39 -|
    //    c3 d4 e4 6b ca 40 9a ac-66 e6 1a a4 1b 39 7e 09 -|
    //    e3 72 99 dd 90 62 55 97-a9 04 c7 51 aa a2 01 cb -|
    //    5a 63 4d 1a e5 99 c3 b1-2a 73 e8 9a 00 46 92 59 -|
    //    39 a3 80 a1 ac 90 52 ea-63 81 49 7d f3 2d 5c c3 -|
    //    19 9f ed fe 81 1d 8c 04-1c d9 23 d2 6d 80 84 f3 -|
    //    00 f2 b1 69 2f cd b3 9f-69 ee 60 3e 4b b5 be 5a -|
    //    09 83 0b bc 3d 3e 05 47-65 96 31 8c 6b c5 e6 a0 -|
    //    00 00 00 00 00 00 00 00                         -/
    //    EncryptedPreMaster::pBlob
    //
    //    The decrypted pre-master data for the above blob is as
    //    cf 7a db cb fb 0e 15 23-87 1c 84 81 ba 9d 4e 15
    //    bb d2 56 bd d8 f7 f3 16-cc 35 3b e1 93 42 78 dd
    //    92 9a e4 7a e2 99 d4 73-b1 aa 6f 55 94 3b c9 bc
    //    0x138: ClientUserName (2 + 2 + 0xe = 0x12 bytes)
    //    0f -\|
    //    00 -/ ClientUserName::wBlobType = BB_CLIENT_USER_NAME

    //    0e -\|
    //    00 -/ ClientUserName::wBlobLen = 0xe bytes

    //    41 64 6d 69 6e 69 73 74 -\|
    //    72 61 74 6f 72 00       -/ ClientUserName::pBlob
    //    0x14a: ClientMachineName (2 + 2 + 7 = 0xb bytes)
    //    10 -\|
    //    00 -/ ClientMachineName::wBlobType = BB_CLIENT_MACHINE_NAME

    //    07 -\|
    //    00 -/ ClientMachineName::wBlobLen = 7 bytes

    //    52 4f 44 45 4e 54 00 -> ClientMachineName::pBlob


    struct NewLicenseRequest_Send
    {
        NewLicenseRequest_Send(OutStream & stream, uint8_t version, const char * username, const char * hostname)
        {
            uint8_t null_data[SEC_MODULUS_SIZE];
            memset(null_data, 0, sizeof(null_data));

            int userlen = strlen(username) + 1;
            int hostlen = strlen(hostname) + 1;
            int length = 128 + userlen + hostlen;

            stream.out_uint8(LIC::NEW_LICENSE_REQUEST);
            stream.out_uint8(version);
            stream.out_uint16_le(length);

            // PreferredKeyExchangeAlg (4 bytes): A 32-bit unsigned integer that
            // indicates the key exchange algorithm chosen by the client. It MUST be set
            // to KEY_EXCHANGE_ALG_RSA (0x00000001), which indicates an RSA-based key
            // exchange with a 512-bit asymmetric key.<9>

            stream.out_uint32_le(LIC::KEY_EXCHANGE_ALG_RSA);

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

            stream.out_uint32_le(0);


            // ClientRandom (32 bytes): A 32-byte random number generated by the client
            // using a cryptographically secure pseudo-random number generator. The
            // ClientRandom and ServerRandom (see section 2.2.2.1) values, along with
            // the data in the EncryptedPreMasterSecret field, are used to generate
            // licensing encryption keys (see section 5.1.3). These keys are used to
            // encrypt licensing protocol messages (see sections 5.1.4 and 5.1.5).

            stream.out_copy_bytes(null_data, SEC_RANDOM_SIZE); // client_random

            // EncryptedPreMasterSecret (variable): A Licensing Binary BLOB structure
            // (see [MS-RDPBCGR] section 2.2.1.12.1.2) of type BB_RANDOM_BLOB (0x0002).
            // This BLOB contains an encrypted 48-byte random number. For instructions
            // on how to encrypt this random number, see section 5.1.2.1.

            // 2.2.1.12.1.2 Licensing Binary Blob (LICENSE_BINARY_BLOB)
            // --------------------------------------------------------
            // The LICENSE_BINARY_BLOB structure is used to encapsulate arbitrary
            // length binary licensing data.

            // wBlobType (2 bytes): A 16-bit, unsigned integer. The data type of
            // the binary information. If wBlobLen is set to 0, then the contents
            // of this field SHOULD be ignored.

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

            // wBlobLen (2 bytes): A 16-bit, unsigned integer. The size in bytes of the
            // binary information in the blobData field. If wBlobLen is set to 0, then the
            // blobData field is not include " in the Licensing Binary BLOB structure and the
            // contents of the wBlobType field SHOULD be ignored.

            // blobData (variable): Variable-length binary data. The size of this data in
            // bytes is given by the wBlobLen field. If wBlobLen is set to 0, then this field
            // is not include " in the Licensing Binary BLOB structure.

            stream.out_uint16_le(LIC::BB_RANDOM_BLOB);
            stream.out_uint16_le((SEC_MODULUS_SIZE + SEC_PADDING_SIZE));
            stream.out_copy_bytes(null_data, SEC_MODULUS_SIZE); // rsa_data
            stream.out_clear_bytes(SEC_PADDING_SIZE);

            // ClientUserName (variable): A Licensing Binary BLOB structure (see
            // [MS-RDPBCGR] section 2.2.1.12.1.2) of type BB_CLIENT_USER_NAME_BLOB
            // (0x000F). This BLOB contains the client user name string in
            // null-terminated ANSI character set format and is used along with the
            // ClientMachineName BLOB to keep track of licenses issued to clients.

            stream.out_uint16_le(LIC::LICENSE_TAG_USER);
            stream.out_uint16_le(userlen);
            stream.out_copy_bytes(username, userlen);

            // ClientMachineName (variable): A Licensing Binary BLOB structure (see
            // [MS-RDPBCGR] section 2.2.1.12.1.2) of type BB_CLIENT_MACHINE_NAME_BLOB
            // (0x0010). This BLOB contains the client machine name string in
            // null-terminated ANSI character set format and is used along with the
            // ClientUserName BLOB to keep track of licenses issued to clients.

            stream.out_uint16_le(LIC::LICENSE_TAG_HOST);
            stream.out_uint16_le(hostlen);
            stream.out_copy_bytes(hostname, hostlen);
        }
    };


    struct NewLicenseRequest_Recv
    {
        uint8_t tag;
        uint8_t flags;
        uint16_t wMsgSize;

        // PreferredKeyExchangeAlg (4 bytes): A 32-bit unsigned integer that
        // indicates the key exchange algorithm chosen by the client. It MUST be set
        // to KEY_EXCHANGE_ALG_RSA (0x00000001), which indicates an RSA-based key
        // exchange with a 512-bit asymmetric key.<9>

        uint32_t dwPreferredKeyExchangeAlg;

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

        uint32_t dwPlatformId;

        // ClientRandom (32 bytes): A 32-byte random number generated by the client
        // using a cryptographically secure pseudo-random number generator. The
        // ClientRandom and ServerRandom (see section 2.2.2.1) values, along with
        // the data in the EncryptedPreMasterSecret field, are used to generate
        // licensing encryption keys (see section 5.1.3). These keys are used to
        // encrypt licensing protocol messages (see sections 5.1.4 and 5.1.5).

        uint8_t client_random[32];

        // EncryptedPreMasterSecret (variable): A Licensing Binary BLOB structure
        // (see [MS-RDPBCGR] section 2.2.1.12.1.2) of type BB_RANDOM_BLOB (0x0002).
        // This BLOB contains an encrypted 48-byte random number. For instructions
        // on how to encrypt this random number, see section 5.1.2.1.

        // 2.2.1.12.1.2 Licensing Binary Blob (LICENSE_BINARY_BLOB)
        // --------------------------------------------------------
        // The LICENSE_BINARY_BLOB structure is used to encapsulate arbitrary
        // length binary licensing data.

        // wBlobType (2 bytes): A 16-bit, unsigned integer. The data type of
        // the binary information. If wBlobLen is set to 0, then the contents
        // of this field SHOULD be ignored.

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

        // wBlobLen (2 bytes): A 16-bit, unsigned integer. The size in bytes of the
        // binary information in the blobData field. If wBlobLen is set to 0, then the
        // blobData field is not include " in the Licensing Binary BLOB structure and the
        // contents of the wBlobType field SHOULD be ignored.

        // blobData (variable): Variable-length binary data. The size of this data in
        // bytes is given by the wBlobLen field. If wBlobLen is set to 0, then this field
        // is not include " in the Licensing Binary BLOB structure.

        // -------------------------------<<<<<<<<<<<<<<<<<

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


        uint16_t license_size;
        uint8_t * license_data;
        uint8_t * hwid;
        uint8_t * signature;

        explicit NewLicenseRequest_Recv(InStream & stream) : license_size(0), license_data(nullptr),
        		hwid(nullptr), signature(nullptr)
        {
            const unsigned expected =
                /* tag(1) + flags(1) + wMsgSize(2) + dwPreferredKeyExchangeAlg(4) + dwPlatformId(4) +
                   client_random(SEC_RANDOM_SIZE) + ignored(2) + lenLicensingBlob(2)
                 */
                12 + SEC_RANDOM_SIZE + 4;
            if (!stream.in_check_rem(expected)){
                LOG(LOG_ERR, "Licence NewLicenseRequest_Recv : Truncated data, need=%u, remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_LIC);
            }

            this->tag = stream.in_uint8();
            this->flags = stream.in_uint8();
            this->wMsgSize = stream.in_uint16_le();

            this->dwPreferredKeyExchangeAlg = stream.in_uint32_le();
            this->dwPlatformId = stream.in_uint32_le();
            stream.in_copy_bytes(this->client_random, SEC_RANDOM_SIZE); // client_random

            // EncryptedPreMasterSecret (variable): A Licensing Binary BLOB structure
            // (see [MS-RDPBCGR] section 2.2.1.12.1.2) of type BB_RANDOM_BLOB (0x0002).
            // This BLOB contains an encrypted 48-byte random number. For instructions
            // on how to encrypt this random number, see section 5.1.2.1.

            // 2.2.1.12.1.2 Licensing Binary Blob (LICENSE_BINARY_BLOB)
            // --------------------------------------------------------
            // The LICENSE_BINARY_BLOB structure is used to encapsulate arbitrary
            // length binary licensing data.

            // wBlobType (2 bytes): A 16-bit, unsigned integer. The data type of
            // the binary information. If wBlobLen is set to 0, then the contents
            // of this field SHOULD be ignored.

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

            // wBlobLen (2 bytes): A 16-bit, unsigned integer. The size in bytes of the
            // binary information in the blobData field. If wBlobLen is set to 0, then the
            // blobData field is not include " in the Licensing Binary BLOB structure and the
            // contents of the wBlobType field SHOULD be ignored.

            // blobData (variable): Variable-length binary data. The size of this data in
            // bytes is given by the wBlobLen field. If wBlobLen is set to 0, then this field
            // is not include " in the Licensing Binary BLOB structure.

            stream.in_skip_bytes(2); /* wBlobType */
            uint16_t lenLicensingBlob = stream.in_uint16_le();

            if (!stream.in_check_rem(lenLicensingBlob)){
                LOG(LOG_ERR, "Licence NewLicenseRequest_Recv : Truncated blobData, need=%" PRIu16 ", remains=%zu",
                    lenLicensingBlob, stream.in_remain());
                throw Error(ERR_LIC);
            }

            stream.in_skip_bytes(lenLicensingBlob); /* blobData */

            // TODO Add missing fields
            //stream.end = stream.p;
        }
    };


//    2.2.2.3 Client License Information (CLIENT_LICENSE_INFO)
//    --------------------------------------------------------

//    The Client License Information packet is sent by a client that already has a license issued to it in response to
// the Server License Request (section 2.2.2.1) message.

// PreferredKeyExchangeAlg (4 bytes): The content and format of this field are the same as the PreferredKeyExchangeAlg
//  field of the Client New License Request (section 2.2.2.2) message.

// PlatformId (4 bytes): The content and format of this field are the same as the PlatformId field of the Client New
//   License Request message.

// ClientRandom (32 bytes): The content and format of this field are the same as the ClientRandom field of the Client
//   New License Request message.

//  EncryptedPreMasterSecret (variable): The content and format of this field are the same as the
//    EncryptedPreMasterSecret field of the Client New License Request message.

//  LicenseInfo (variable): A Licensing Binary BLOB structure (see [MS-RDPBCGR] section 2.2.1.12.1.2) of type
//    BB_DATA_BLOB (0x0001). This BLOB contains the CAL (see the pbLicenseInfo field in section 2.2.2.6.1) that is
//    retrieved from the client's license store.

//  EncryptedHWID (variable): A Licensing Binary BLOB structure (see [MS-RDPBCGR] section 2.2.1.12.1.2). This BLOB
//    contains a Client Hardware Identification (section 2.2.2.3.1) structure encrypted with the licensing encryption
//    keys (see section 5.1.3), using RC4 (for instructions on how to perform the encryption, see section 5.1.4).

// MACData (16 bytes): An array of 16 bytes containing an MD5 digest (Message Authentication Code (MAC)) that is
//    generated over the unencrypted Client Hardware Identification structure. For instructions on how to generate this
//    message digest, see section 5.1.6; for a description of how the server uses the MACData field to verify the
//    integrity of the Client Hardware Identification structure, see section 3.1.5.1.

    //    4.3 CLIENT LICENSE INFO
    //    -----------------------

    //    If the client already has a license in its store, it sends the license in the Client License Information message as a response to the Server License Request message. See sections 2.2.2.3 and 3.3.5.3.

    //    00000000:  12 83 fd 08 01 00 00 00-00 00 01 04 26 c9 32 34  ............&.24
    //    00000010:  7d 2b e1 75 50 5e 47 7e-76 8d 78 7b bb 21 aa b7  }+.uP^G~v.x{.!..
    //    00000020:  b0 b8 ea 6c dd c1 b0 01-e6 13 be d8 02 00 08 01  ...l............
    //    00000030:  b1 fa 1c 25 d9 5e 9d 04-cd d2 d4 34 c6 a2 e6 f3  ...%.^.....4....
    //    00000040:  a2 bf bc 53 8a 0e 15 cf-1c 1a 99 bb 49 dc 9e 71  ...S........I..q
    //    00000050:  39 03 78 ce 5d 72 29 09-3a 86 b1 1d e8 b1 38 07  9.x.]r).:.....8.
    //    00000060:  29 62 87 84 1e cc 95 b0-49 19 a0 cf 27 90 9a ef  )b......I...'...
    //    00000070:  d6 a9 9a f4 66 d9 d2 e9-64 ee 4a aa e4 22 d6 44  ....f...d.J..".D
    //    00000080:  b8 72 79 3a d2 82 09 e1-1f 85 69 3e 09 68 c3 fa  .ry:......i>.h..
    //    00000090:  36 e2 ba 3c c5 4c 46 7d-68 42 2d 03 bf e0 03 d4  6..<.LF}hB-.....
    //    000000A0:  13 45 da 21 14 32 41 86-c4 00 d0 42 ef 68 ee 9c  .E.!.2A....B.h..
    //    000000B0:  0a 1a d9 42 6f 64 7b e4-b9 61 db be 52 e6 3e 63  ...Bod{..a..R.>c
    //    000000C0:  dc ec 7e 97 a0 0d 87 82-be 56 be 29 8b 7d 27 bd  ..~......V.).}'.
    //    000000D0:  2a 31 57 b0 3a 3a ad 35-70 d4 bd 29 20 8d 82 dc  *1W.::.5p..) ...
    //    000000E0:  f5 7c f9 3a 2f 0d a2 5b-55 7a 95 37 af d5 d8 47  .|.:/..[Uz.7...G
    //    000000F0:  f9 1d a3 89 5a cf 66 b2-4d b2 83 db c2 45 c8 3b  ....Z.f.M....E.;
    //    00000100:  86 31 1c 2f a7 59 1a 09-89 56 ed 18 09 4c 35 1a  .1./.Y...V...L5.
    //    00000110:  47 5c b3 35 f2 09 01 bf-6a 02 bc fc be 75 a6 8c  G\.5....j....u..
    //    00000120:  70 2e 3b 03 73 64 b4 13-89 0c 1e a4 3e 49 e9 b9  p.;.sd......>I..
    //    00000130:  00 00 00 00 00 00 00 00-01 00 99 07 30 82 07 95  ............0...
    //    00000140:  06 09 2a 86 48 86 f7 0d-01 07 02 a0 82 07 86 30  ..*.H..........0
    //    00000150:  82 07 82 02 01 01 31 00-30 0b 06 09 2a 86 48 86  ......1.0...*.H.
    //    00000160:  f7 0d 01 07 01 a0 82 07-6a 30 82 02 f1 30 82 01  ........j0...0..
    //    00000170:  dd a0 03 02 01 02 02 08-01 9e 27 4d 68 ac ed 20  ..........'Mh..
    //    00000180:  30 09 06 05 2b 0e 03 02-1d 05 00 30 32 31 30 30  0...+......02100
    //    00000190:  13 06 03 55 04 03 1e 0c-00 52 00 4f 00 44 00 45  ...U.....R.O.D.E
    //    000001A0:  00 4e 00 54 30 19 06 03-55 04 07 1e 12 00 57 00  .N.T0...U.....W.
    //    000001B0:  4f 00 52 00 4b 00 47 00-52 00 4f 00 55 00 50 30  O.R.K.G.R.O.U.P0
    //    000001C0:  1e 17 0d 37 30 30 35 33-30 31 30 33 36 31 38 5a  ...700530103618Z
    //    000001D0:  17 0d 34 39 30 35 33 30-31 30 33 36 31 38 5a 30  ..490530103618Z0
    //    000001E0:  32 31 30 30 13 06 03 55-04 03 1e 0c 00 52 00 4f  2100...U.....R.O
    //    000001F0:  00 44 00 45 00 4e 00 54-30 19 06 03 55 04 07 1e  .D.E.N.T0...U...
    //    00000200:  12 00 57 00 4f 00 52 00-4b 00 47 00 52 00 4f 00  ..W.O.R.K.G.R.O.
    //    00000210:  55 00 50 30 82 01 22 30-0d 06 09 2a 86 48 86 f7  U.P0.."0...*.H..
    //    00000220:  0d 01 01 01 05 00 03 82-01 0f 00 30 82 01 0a 02  ...........0....
    //    00000230:  82 01 01 00 88 ad 7c 8f-8b 82 76 5a bd 8f 6f 62  ......|...vZ..ob
    //    00000240:  18 e1 d9 aa 41 fd ed 68-01 c6 34 35 b0 29 04 ca  ....A..h..45.)..
    //    00000250:  4a 4a 1c 7e 80 14 f7 8e-77 b8 25 ff 16 47 6f bd  JJ.~....w.%..Go.
    //    00000260:  e2 34 3d 2e 02 b9 53 e4-33 75 ad 73 28 80 a0 4d  .4=...S.3u.s(..M
    //    00000270:  fc 6c c0 22 53 1b 2c f8-f5 01 60 19 7e 79 19 39  .l."S.,...`.~y.9
    //    00000280:  8d b5 ce 39 58 dd 55 24-3b 55 7b 43 c1 7f 14 2f  ...9X.U$;U{C.../
    //    00000290:  b0 64 3a 54 95 2b 88 49-0c 61 2d ac f8 45 f5 da  .d:T.+.I.a-..E..
    //    000002A0:  88 18 5f ae 42 f8 75 c7-26 6d b5 bb 39 6f cc 55  .._.B.u.&m..9o.U
    //    000002B0:  1b 32 11 38 8d e4 e9 44-84 11 36 a2 61 76 aa 4c  .2.8...D..6.av.L
    //    000002C0:  b4 e3 55 0f e4 77 8e de-e3 a9 ea b7 41 94 00 58  ..U..w......A..X
    //    000002D0:  aa c9 34 a2 98 c6 01 1a-76 14 01 a8 dc 30 7c 77  ..4.....v....0|w
    //    000002E0:  5a 20 71 5a a2 3f af 13-7e e8 fd 84 a2 5b cf 25  Z qZ.?..~....[.%
    //    000002F0:  e9 c7 8f a8 f2 8b 84 c7-04 5e 53 73 4e 0e 89 a3  .........^SsN...
    //    00000300:  3c e7 68 5c 24 b7 80 53-3c 54 c8 c1 53 aa 71 71  <.h\$..S<T..S.qq
    //    00000310:  3d 36 15 d6 6a 9d 7d de-ae f9 e6 af 57 ae b9 01  =6..j.}.....W...
    //    00000320:  96 5d e0 4d cd ed c8 d7-f3 01 03 38 10 be 7c 42  .].M.......8..|B
    //    00000330:  67 01 a7 23 02 03 01 00-01 a3 13 30 11 30 0f 06  g..#.......0.0..
    //    00000340:  03 55 1d 13 04 08 30 06-01 01 ff 02 01 00 30 09  .U....0.......0.
    //    00000350:  06 05 2b 0e 03 02 1d 05-00 03 82 01 01 00 70 db  ..+...........p.
    //    00000360:  21 2b 84 9a 7a c3 b1 68-fa c0 00 8b 71 ab 43 9f  !+..z..h....q.C.
    //    00000370:  b6 7b b7 1f 20 83 ac 0a-b5 0e ad b6 36 ef 65 17  .{.. .......6.e.
    //    00000380:  99 86 8a 3d ba 0c 53 2e-a3 75 a0 f3 11 3d e7 65  ...=..S..u...=.e
    //    00000390:  4b ae 3c 42 70 11 dc ca-83 c0 be 3e 97 71 84 69  K.<Bp......>.q.i
    //    000003A0:  d6 a8 27 33 9b 3e 17 3c-a0 4c 64 ca 20 37 a4 11  ..'3.>.<.Ld. 7..
    //    000003B0:  a9 28 8f b7 18 96 69 15-0d 74 04 75 2a 00 c7 a6  .(....i..t.u*...
    //    000003C0:  6a be ac b3 f2 fb 06 1b-6c 11 bd 96 e2 34 74 5d  j.......l....4t]
    //    000003D0:  f5 98 8f 3a 8d 69 08 6f-53 12 4e 39 80 90 ce 8b  ...:.i.oS.N9....
    //    000003E0:  5e 88 23 2d fd 55 fd 58-3d 39 27 b3 7c 57 fe 3b  ^.#-.U.X=9'.|W.;
    //    000003F0:  ab 62 26 60 e2 d0 c8 f4-02 23 16 c3 52 5d 9f 05  .b&`.....#..R]..
    //    00000400:  49 a2 71 2d 6d 5b 90 dd-bf e5 a9 2e f1 85 8a 8a  I.q-m[..........
    //    00000410:  b8 a9 6b 13 cc 8d 4c 22-41 ad 32 1e 3b 4b 89 37  ..k...L"A.2.;K.7
    //    00000420:  66 df 1e a5 4a 03 52 1c-d9 19 79 22 d4 a7 3b 47  f...J.R...y"..;G
    //    00000430:  93 a9 0c 03 6a d8 5f fc-c0 75 33 e5 26 da f7 4a  ....j._..u3.&..J
    //    00000440:  77 d8 f1 30 80 39 38 1e-86 1d 97 00 9c 0e ba 00  w..0.98.........
    //    00000450:  54 8a c0 12 32 6f 3d c4-15 f9 50 f8 ce 95 30 82  T...2o=...P...0.
    //    00000460:  04 71 30 82 03 5d a0 03-02 01 02 02 05 03 00 00  .q0..]..........
    //    00000470:  00 0f 30 09 06 05 2b 0e-03 02 1d 05 00 30 32 31  ..0...+......021
    //    00000480:  30 30 13 06 03 55 04 03-1e 0c 00 52 00 4f 00 44  00...U.....R.O.D
    //    00000490:  00 45 00 4e 00 54 30 19-06 03 55 04 07 1e 12 00  .E.N.T0...U.....
    //    000004A0:  57 00 4f 00 52 00 4b 00-47 00 52 00 4f 00 55 00  W.O.R.K.G.R.O.U.
    //    000004B0:  50 30 1e 17 0d 30 37 30-36 32 30 31 34 35 31 33  P0...07062014513
    //    000004C0:  35 5a 17 0d 30 37 30 39-31 38 31 34 35 31 33 35  5Z..070918145135
    //    000004D0:  5a 30 7f 31 7d 30 13 06-03 55 04 03 1e 0c 00 52  Z0.1}0...U.....R
    //    000004E0:  00 4f 00 44 00 45 00 4e-00 54 30 21 06 03 55 04  .O.D.E.N.T0!..U.
    //    000004F0:  07 1e 1a 00 41 00 64 00-6d 00 69 00 6e 00 69 00  ....A.d.m.i.n.i.
    //    00000500:  73 00 74 00 72 00 61 00-74 00 6f 00 72 30 43 06  s.t.r.a.t.o.r0C.
    //    00000510:  03 55 04 05 1e 3c 00 31-00 42 00 63 00 4b 00 65  .U...<.1.B.c.K.e
    //    00000520:  00 64 00 79 00 32 00 6b-00 72 00 4f 00 34 00 2f  .d.y.2.k.r.O.4./
    //    00000530:  00 4d 00 43 00 44 00 4c-00 49 00 31 00 41 00 48  .M.C.D.L.I.1.A.H
    //    00000540:  00 5a 00 63 00 50 00 69-00 61 00 73 00 3d 00 0d  .Z.c.P.i.a.s.=..
    //    00000550:  00 0a 30 82 01 22 30 0d-06 09 2a 86 48 86 f7 0d  ..0.."0...*.H...
    //    00000560:  01 01 01 05 00 03 82 01-0f 00 30 82 01 0a 02 82  ..........0.....
    //    00000570:  01 01 00 88 ad 7c 8f 8b-82 76 5a bd 8f 6f 62 18  .....|...vZ..ob.
    //    00000580:  e1 d9 aa 41 fd ed 68 01-c6 34 35 b0 29 04 ca 4a  ...A..h..45.)..J
    //    00000590:  4a 1c 7e 80 14 f7 8e 77-b8 25 ff 16 47 6f bd e2  J.~....w.%..Go..
    //    000005A0:  34 3d 2e 02 b9 53 e4 33-75 ad 73 28 80 a0 4d fc  4=...S.3u.s(..M.
    //    000005B0:  6c c0 22 53 1b 2c f8 f5-01 60 19 7e 79 19 39 8d  l."S.,...`.~y.9.
    //    000005C0:  b5 ce 39 58 dd 55 24 3b-55 7b 43 c1 7f 14 2f b0  ..9X.U$;U{C.../.
    //    000005D0:  64 3a 54 95 2b 88 49 0c-61 2d ac f8 45 f5 da 88  d:T.+.I.a-..E...
    //    000005E0:  18 5f ae 42 f8 75 c7 26-6d b5 bb 39 6f cc 55 1b  ._.B.u.&m..9o.U.
    //    000005F0:  32 11 38 8d e4 e9 44 84-11 36 a2 61 76 aa 4c b4  2.8...D..6.av.L.
    //    00000600:  e3 55 0f e4 77 8e de e3-a9 ea b7 41 94 00 58 aa  .U..w......A..X.
    //    00000610:  c9 34 a2 98 c6 01 1a 76-14 01 a8 dc 30 7c 77 5a  .4.....v....0|wZ
    //    00000620:  20 71 5a a2 3f af 13 7e-e8 fd 84 a2 5b cf 25 e9  qZ.?..~....[.%.
    //    00000630:  c7 8f a8 f2 8b 84 c7 04-5e 53 73 4e 0e 89 a3 3c  ........^SsN...<
    //    00000640:  e7 68 5c 24 b7 80 53 3c-54 c8 c1 53 aa 71 71 3d  .h\$..S<T..S.qq=
    //    00000650:  36 15 d6 6a 9d 7d de ae-f9 e6 af 57 ae b9 01 96  6..j.}.....W....
    //    00000660:  5d e0 4d cd ed c8 d7 f3-01 03 38 10 be 7c 42 67  ].M.......8..|Bg
    //    00000670:  01 a7 23 02 03 01 00 01-a3 82 01 47 30 82 01 43  ..#........G0..C
    //    00000680:  30 14 06 09 2b 06 01 04-01 82 37 12 04 01 01 ff  0...+.....7.....
    //    00000690:  04 04 01 00 05 00 30 3c-06 09 2b 06 01 04 01 82  ......0<..+.....
    //    000006A0:  37 12 02 01 01 ff 04 2c-4d 00 69 00 63 00 72 00  7......,M.i.c.r.
    //    000006B0:  6f 00 73 00 6f 00 66 00-74 00 20 00 43 00 6f 00  o.s.o.f.t. .C.o.
    //    000006C0:  72 00 70 00 6f 00 72 00-61 00 74 00 69 00 6f 00  r.p.o.r.a.t.i.o.
    //    000006D0:  6e 00 00 00 30 56 06 09-2b 06 01 04 01 82 37 12  n...0V..+.....7.
    //    000006E0:  05 01 01 ff 04 46 00 30-00 00 01 00 00 00 ff 00  .....F.0........
    //    000006F0:  00 00 00 04 00 00 1c 00-08 00 24 00 16 00 3a 00  ..........$...:.
    //    00000700:  01 00 41 00 30 00 32 00-00 00 41 00 30 00 32 00  ..A.0.2...A.0.2.
    //    00000710:  2d 00 36 00 2e 00 30 00-30 00 2d 00 53 00 00 00  -.6...0.0.-.S...
    //    00000720:  06 00 00 00 00 80 64 80-00 00 00 00 30 6e 06 09  ......d.....0n..
    //    00000730:  2b 06 01 04 01 82 37 12-06 01 01 ff 04 5e 00 30  +.....7......^.0
    //    00000740:  00 00 00 00 0e 00 3e 00-52 00 4f 00 44 00 45 00  ......>.R.O.D.E.
    //    00000750:  4e 00 54 00 00 00 37 00-38 00 34 00 34 00 30 00  N.T...7.8.4.4.0.
    //    00000760:  2d 00 30 00 30 00 36 00-2d 00 35 00 38 00 36 00  -.0.0.6.-.5.8.6.
    //    00000770:  37 00 30 00 34 00 35 00-2d 00 37 00 30 00 33 00  7.0.4.5.-.7.0.3.
    //    00000780:  34 00 37 00 00 00 57 00-4f 00 52 00 4b 00 47 00  4.7...W.O.R.K.G.
    //    00000790:  52 00 4f 00 55 00 50 00-00 00 00 00 30 25 06 03  R.O.U.P.....0%..
    //    000007A0:  55 1d 23 01 01 ff 04 1b-30 19 a1 10 a4 0e 52 00  U.#.....0.....R.
    //    000007B0:  4f 00 44 00 45 00 4e 00-54 00 00 00 82 05 03 00  O.D.E.N.T.......
    //    000007C0:  00 00 0f 30 09 06 05 2b-0e 03 02 1d 05 00 03 82  ...0...+........
    //    000007D0:  01 01 00 13 1b dc 89 d2-fc 54 0c ee 82 45 68 6a  .........T...Ehj
    //    000007E0:  72 c3 3e 17 73 96 53 44-39 50 0e 0b 9f 95 d6 2c  r.>.s.SD9P.....,
    //    000007F0:  6b 53 14 9c e5 55 ed 65-df 2a eb 5c 64 85 70 1f  kS...U.e.*.\d.p.
    //    00000800:  bc 96 cf a3 76 b1 72 3b-e1 f6 ad ad ad 2a 14 af  ....v.r;.....*..
    //    00000810:  ba d0 d6 d5 6d 55 ec 1e-c3 4b ba 06 9c 59 78 93  ....mU...K...Yx.
    //    00000820:  64 87 4b 03 f9 ee 4c dd-36 5b bd d4 e5 4c 4e da  d.K...L.6[...LN.
    //    00000830:  7b c1 ae 23 28 9e 77 6f-0f e6 94 fe 05 22 00 ab  {..#(.wo....."..
    //    00000840:  63 5b e1 82 45 a6 ec 1f-6f 2c 7b 56 de 78 25 7d  c[..E...o,{V.x%}
    //    00000850:  10 60 0e 53 42 4b 6c 7a-6b 5d c9 d5 a6 ae c8 c8  .`.SBKlzk]......
    //    00000860:  52 29 d6 42 56 02 ec f9-23 a8 8c 8d 89 c9 7c 84  R).BV...#.....|.
    //    00000870:  07 fc 33 e1 1e ea e2 8f-2b be 8f a9 d3 d1 e1 5e  ..3.....+......^
    //    00000880:  0b dc b6 43 6e 33 0a f4-2e 9d 0c c9 58 54 34 aa  ...Cn3......XT4.
    //    00000890:  e1 d2 a2 e4 90 02 23 26-a0 92 26 26 0a 83 b4 4d  ......#&..&&...M
    //    000008A0:  d9 4b ef eb 9d a9 24 3f-92 8b db 04 7b 9d 64 91  .K....$?....{.d.
    //    000008B0:  a4 4b d2 6e 51 05 08 c9-91 af 31 26 55 21 b1 ea  .K.nQ.....1&U!..
    //    000008C0:  ce a3 a4 0d 5e 4c 46 db-16 2d 98 dc 60 19 b8 1b  ....^LF..-..`...
    //    000008D0:  b9 cd fb 31 00 01 00 14-00 b9 30 59 3b 93 61 c9  ...1......0Y;.a.
    //    000008E0:  f6 b6 0b 1f dc 1a 85 67-39 dc 29 65 62 42 a2 13  .......g9.)ebB..
    //    000008F0:  c7 54 ae b5 d5 24 66 54-f3 1b af 8d fb           .T...$fT.....

    //    0x00: LICENSE_PREAMBLE (4 bytes)
    //                               12 -> LICENSE_PREAMBLE::bMsgType = CLIENT_LICENSE_INFO
    //
    //    83 -> LICENSE_PREAMBLE::bVersion = 0x80 | 0x3
    //

    //    fd -\|
    //    08 -/ LICENSE_PREAMBLE::wMsgSize = 0x8fd bytes
    //
    //    0x04: PreferredKeyExchgAlg (4 bytes)
    //    01 -\|
    //    00 -|
    //    00 -|
    //    00 -/ CLIENT_LICENSE_INFO::dwPrefKeyExchangeAlg = 1
    //
    //    0x08: PlatformId (4 bytes)
    //    00 -\|
    //    00 -|
    //    01 -|
    //    04 -/ CLIENT_LICENSE_INFO::dwPlatformId = 0x04010000
    //    0x0c: Client Random (0x20 bytes)
    //             26 c9 32 34 7d 2b e1 75 -\|
    //    50 5e 47 7e-76 8d 78 7b -|
    //    bb 21 aa b7 b0 b8 ea 6c -|
    //    dd c1 b0 01-e6 13 be d8 -/ CLIENT_LICENSE_INFO::ClientRandom
    //    0x2c: EncryptedPreMaster (2 + 2 + 0x108 = 0x10c bytes)
    //    02 -\|
    //    00 -/ EncryptedPreMasterSecret::wBlobType

    //    08 -\|
    //    01 -/ EncryptedPreMasterSecret::wBlobLen

    //    b1 fa 1c 25 d9 5e 9d 04-cd d2 d4 34 c6 a2 e6 f3 -\|
    //    a2 bf bc 53 8a 0e 15 cf-1c 1a 99 bb 49 dc 9e 71 -|
    //    39 03 78 ce 5d 72 29 09-3a 86 b1 1d e8 b1 38 07 -|
    //    29 62 87 84 1e cc 95 b0-49 19 a0 cf 27 90 9a ef -|
    //    d6 a9 9a f4 66 d9 d2 e9-64 ee 4a aa e4 22 d6 44 -|
    //    b8 72 79 3a d2 82 09 e1-1f 85 69 3e 09 68 c3 fa -|
    //    36 e2 ba 3c c5 4c 46 7d-68 42 2d 03 bf e0 03 d4 -|
    //    13 45 da 21 14 32 41 86-c4 00 d0 42 ef 68 ee 9c -|
    //    0a 1a d9 42 6f 64 7b e4-b9 61 db be 52 e6 3e 63 -|
    //    dc ec 7e 97 a0 0d 87 82-be 56 be 29 8b 7d 27 bd -|
    //    2a 31 57 b0 3a 3a ad 35-70 d4 bd 29 20 8d 82 dc -|
    //    f5 7c f9 3a 2f 0d a2 5b-55 7a 95 37 af d5 d8 47 -|
    //    f9 1d a3 89 5a cf 66 b2-4d b2 83 db c2 45 c8 3b -|
    //    86 31 1c 2f a7 59 1a 09-89 56 ed 18 09 4c 35 1a -|
    //    47 5c b3 35 f2 09 01 bf-6a 02 bc fc be 75 a6 8c -|
    //    70 2e 3b 03 73 64 b4 13-89 0c 1e a4 3e 49 e9 b9 -|
    //    00 00 00 00 00 00 00 00                         -/
    //    EncryptedPreMasterSecret::pBlob
    //    0x138: LicenseInfo (2 + 2 + 0x0799 = 0x79d bytes)
    //    01 -\|
    //    00 -/ LicenseInfo::wBlobType = BB_DATA_BLOB

    //    99 -\|
    //    07 -/ LicenseInfo::wBlobLen = 0x799

    //                                        30 82 07 95 -\|
    //    06 09 2a 86 48 86 f7 0d-01 07 02 a0 82 07 86 30 -|
    //    82 07 82 02 01 01 31 00-30 0b 06 09 2a 86 48 86 -|
    //    f7 0d 01 07 01 a0 82 07-6a 30 82 02 f1 30 82 01 -|
    //    dd a0 03 02 01 02 02 08-01 9e 27 4d 68 ac ed 20 -|
    //    30 09 06 05 2b 0e 03 02-1d 05 00 30 32 31 30 30 -|
    //    13 06 03 55 04 03 1e 0c-00 52 00 4f 00 44 00 45 -|
    //    00 4e 00 54 30 19 06 03-55 04 07 1e 12 00 57 00 -|
    //    4f 00 52 00 4b 00 47 00-52 00 4f 00 55 00 50 30
    //    1e 17 0d 37 30 30 35 33-30 31 30 33 36 31 38 5a -|
    //    17 0d 34 39 30 35 33 30-31 30 33 36 31 38 5a 30 -|
    //    32 31 30 30 13 06 03 55-04 03 1e 0c 00 52 00 4f -|
    //    00 44 00 45 00 4e 00 54-30 19 06 03 55 04 07 1e -|
    //    12 00 57 00 4f 00 52 00-4b 00 47 00 52 00 4f 00 -|
    //    55 00 50 30 82 01 22 30-0d 06 09 2a 86 48 86 f7 -|
    //    0d 01 01 01 05 00 03 82-01 0f 00 30 82 01 0a 02 -|
    //    82 01 01 00 88 ad 7c 8f-8b 82 76 5a bd 8f 6f 62 -|
    //    18 e1 d9 aa 41 fd ed 68-01 c6 34 35 b0 29 04 ca -|
    //    4a 4a 1c 7e 80 14 f7 8e-77 b8 25 ff 16 47 6f bd -|
    //    e2 34 3d 2e 02 b9 53 e4-33 75 ad 73 28 80 a0 4d -|
    //    fc 6c c0 22 53 1b 2c f8-f5 01 60 19 7e 79 19 39 -|
    //    8d b5 ce 39 58 dd 55 24-3b 55 7b 43 c1 7f 14 2f -|
    //    b0 64 3a 54 95 2b 88 49-0c 61 2d ac f8 45 f5 da -|
    //    88 18 5f ae 42 f8 75 c7-26 6d b5 bb 39 6f cc 55 -|
    //    1b 32 11 38 8d e4 e9 44-84 11 36 a2 61 76 aa 4c -|
    //    b4 e3 55 0f e4 77 8e de-e3 a9 ea b7 41 94 00 58 -|
    //    aa c9 34 a2 98 c6 01 1a-76 14 01 a8 dc 30 7c 77 -|
    //    5a 20 71 5a a2 3f af 13-7e e8 fd 84 a2 5b cf 25 -|
    //    e9 c7 8f a8 f2 8b 84 c7-04 5e 53 73 4e 0e 89 a3 -|
    //    3c e7 68 5c 24 b7 80 53-3c 54 c8 c1 53 aa 71 71 -|
    //    3d 36 15 d6 6a 9d 7d de-ae f9 e6 af 57 ae b9 01 -|
    //    96 5d e0 4d cd ed c8 d7-f3 01 03 38 10 be 7c 42 -|
    //    67 01 a7 23 02 03 01 00-01 a3 13 30 11 30 0f 06 -|
    //    03 55 1d 13 04 08 30 06-01 01 ff 02 01 00 30 09 -|
    //    06 05 2b 0e 03 02 1d 05-00 03 82 01 01 00 70 db -|
    //    21 2b 84 9a 7a c3 b1 68-fa c0 00 8b 71 ab 43 9f -|
    //    b6 7b b7 1f 20 83 ac 0a-b5 0e ad b6 36 ef 65 17 -|
    //    99 86 8a 3d ba 0c 53 2e-a3 75 a0 f3 11 3d e7 65 -|
    //    4b ae 3c 42 70 11 dc ca-83 c0 be 3e 97 71 84 69 -|
    //    d6 a8 27 33 9b 3e 17 3c-a0 4c 64 ca 20 37 a4 11 -|
    //    a9 28 8f b7 18 96 69 15-0d 74 04 75 2a 00 c7 a6 -|
    //    6a be ac b3 f2 fb 06 1b-6c 11 bd 96 e2 34 74 5d -|
    //    f5 98 8f 3a 8d 69 08 6f-53 12 4e 39 80 90 ce 8b -|
    //    5e 88 23 2d fd 55 fd 58-3d 39 27 b3 7c 57 fe 3b -|
    //    ab 62 26 60 e2 d0 c8 f4-02 23 16 c3 52 5d 9f 05 -|
    //    49 a2 71 2d 6d 5b 90 dd-bf e5 a9 2e f1 85 8a 8a -|
    //    b8 a9 6b 13 cc 8d 4c 22-41 ad 32 1e 3b 4b 89 37 -|
    //    66 df 1e a5 4a 03 52 1c-d9 19 79 22 d4 a7 3b 47 -|
    //    93 a9 0c 03 6a d8 5f fc-c0 75 33 e5 26 da f7 4a -|
    //    77 d8 f1 30 80 39 38 1e-86 1d 97 00 9c 0e ba 00 -|
    //    54 8a c0 12 32 6f 3d c4-15 f9 50 f8 ce 95 30 82 -|
    //    04 71 30 82 03 5d a0 03-02 01 02 02 05 03 00 00 -|
    //    00 0f 30 09 06 05 2b 0e-03 02 1d 05 00 30 32 31 -|
    //    30 30 13 06 03 55 04 03-1e 0c 00 52 00 4f 00 44 -|
    //    00 45 00 4e 00 54 30 19-06 03 55 04 07 1e 12 00 -|
    //    57 00 4f 00 52 00 4b 00-47 00 52 00 4f 00 55 00 -|
    //    50 30 1e 17 0d 30 37 30-36 32 30 31 34 35 31 33 -|
    //    35 5a 17 0d 30 37 30 39-31 38 31 34 35 31 33 35 -|
    //    5a 30 7f 31 7d 30 13 06-03 55 04 03 1e 0c 00 52 -|
    //    00 4f 00 44 00 45 00 4e-00 54 30 21 06 03 55 04 -|
    //    07 1e 1a 00 41 00 64 00-6d 00 69 00 6e 00 69 00 -|
    //    73 00 74 00 72 00 61 00-74 00 6f 00 72 30 43 06 -|
    //    03 55 04 05 1e 3c 00 31-00 42 00 63 00 4b 00 65 -|
    //    00 64 00 79 00 32 00 6b-00 72 00 4f 00 34 00 2f -|
    //    00 4d 00 43 00 44 00 4c-00 49 00 31 00 41 00 48 -|
    //    00 5a 00 63 00 50 00 69-00 61 00 73 00 3d 00 0d -|
    //    00 0a 30 82 01 22 30 0d-06 09 2a 86 48 86 f7 0d -|
    //    01 01 01 05 00 03 82 01-0f 00 30 82 01 0a 02 82 -|
    //    01 01 00 88 ad 7c 8f 8b-82 76 5a bd 8f 6f 62 18 -|
    //    e1 d9 aa 41 fd ed 68 01-c6 34 35 b0 29 04 ca 4a -|
    //    4a 1c 7e 80 14 f7 8e 77-b8 25 ff 16 47 6f bd e2 -|
    //    34 3d 2e 02 b9 53 e4 33-75 ad 73 28 80 a0 4d fc -|
    //    6c c0 22 53 1b 2c f8 f5-01 60 19 7e 79 19 39 8d -|
    //    b5 ce 39 58 dd 55 24 3b-55 7b 43 c1 7f 14 2f b0 -|
    //    64 3a 54 95 2b 88 49 0c-61 2d ac f8 45 f5 da 88 -|
    //    18 5f ae 42 f8 75 c7 26-6d b5 bb 39 6f cc 55 1b -|
    //    32 11 38 8d e4 e9 44 84-11 36 a2 61 76 aa 4c b4 -|
    //    e3 55 0f e4 77 8e de e3-a9 ea b7 41 94 00 58 aa -|
    //    c9 34 a2 98 c6 01 1a 76-14 01 a8 dc 30 7c 77 5a -|
    //    20 71 5a a2 3f af 13 7e-e8 fd 84 a2 5b cf 25 e9 -|
    //    c7 8f a8 f2 8b 84 c7 04-5e 53 73 4e 0e 89 a3 3c -|
    //    e7 68 5c 24 b7 80 53 3c-54 c8 c1 53 aa 71 71 3d -|
    //    36 15 d6 6a 9d 7d de ae-f9 e6 af 57 ae b9 01 96 -|
    //    5d e0 4d cd ed c8 d7 f3-01 03 38 10 be 7c 42 67 -|
    //    01 a7 23 02 03 01 00 01-a3 82 01 47 30 82 01 43 -|
    //    30 14 06 09 2b 06 01 04-01 82 37 12 04 01 01 ff -|
    //    04 04 01 00 05 00 30 3c-06 09 2b 06 01 04 01 82 -|
    //    37 12 02 01 01 ff 04 2c-4d 00 69 00 63 00 72 00 -|
    //    6f 00 73 00 6f 00 66 00-74 00 20 00 43 00 6f 00 -|
    //    72 00 70 00 6f 00 72 00-61 00 74 00 69 00 6f 00 -|
    //    6e 00 00 00 30 56 06 09-2b 06 01 04 01 82 37 12 -|
    //    05 01 01 ff 04 46 00 30-00 00 01 00 00 00 ff 00 -|
    //    00 00 00 04 00 00 1c 00-08 00 24 00 16 00 3a 00 -|
    //    01 00 41 00 30 00 32 00-00 00 41 00 30 00 32 00 -|
    //    2d 00 36 00 2e 00 30 00-30 00 2d 00 53 00 00 00 -|
    //    06 00 00 00 00 80 64 80-00 00 00 00 30 6e 06 09 -|
    //    2b 06 01 04 01 82 37 12-06 01 01 ff 04 5e 00 30 -|
    //    00 00 00 00 0e 00 3e 00-52 00 4f 00 44 00 45 00 -|
    //    4e 00 54 00 00 00 37 00-38 00 34 00 34 00 30 00 -|
    //    2d 00 30 00 30 00 36 00-2d 00 35 00 38 00 36 00 -|
    //    37 00 30 00 34 00 35 00-2d 00 37 00 30 00 33 00 -|
    //    34 00 37 00 00 00 57 00-4f 00 52 00 4b 00 47 00 -|
    //    52 00 4f 00 55 00 50 00-00 00 00 00 30 25 06 03 -|
    //    55 1d 23 01 01 ff 04 1b-30 19 a1 10 a4 0e 52 00 -|
    //    4f 00 44 00 45 00 4e 00-54 00 00 00 82 05 03 00 -|
    //    00 00 0f 30 09 06 05 2b-0e 03 02 1d 05 00 03 82 -|
    //    01 01 00 13 1b dc 89 d2-fc 54 0c ee 82 45 68 6a -|
    //    72 c3 3e 17 73 96 53 44-39 50 0e 0b 9f 95 d6 2c -|
    //    6b 53 14 9c e5 55 ed 65-df 2a eb 5c 64 85 70 1f -|
    //    bc 96 cf a3 76 b1 72 3b-e1 f6 ad ad ad 2a 14 af -|
    //    ba d0 d6 d5 6d 55 ec 1e-c3 4b ba 06 9c 59 78 93 -|
    //    64 87 4b 03 f9 ee 4c dd-36 5b bd d4 e5 4c 4e da -|
    //    7b c1 ae 23 28 9e 77 6f-0f e6 94 fe 05 22 00 ab -|
    //    63 5b e1 82 45 a6 ec 1f-6f 2c 7b 56 de 78 25 7d -|
    //    10 60 0e 53 42 4b 6c 7a-6b 5d c9 d5 a6 ae c8 c8 -|
    //    52 29 d6 42 56 02 ec f9-23 a8 8c 8d 89 c9 7c 84 -|
    //    07 fc 33 e1 1e ea e2 8f-2b be 8f a9 d3 d1 e1 5e -|
    //    0b dc b6 43 6e 33 0a f4-2e 9d 0c c9 58 54 34 aa -|
    //    e1 d2 a2 e4 90 02 23 26-a0 92 26 26 0a 83 b4 4d -|
    //    d9 4b ef eb 9d a9 24 3f-92 8b db 04 7b 9d 64 91 -|
    //    a4 4b d2 6e 51 05 08 c9-91 af 31 26 55 21 b1 ea -|
    //    ce a3 a4 0d 5e 4c 46 db-16 2d 98 dc 60 19 b8 1b -|
    //    b9 cd fb 31 00                                  -/
    //    0x8d5: EncryptedHWID (2 + 2 + 0x14 = 0x18 bytes)
    //    01 -\|
    //    00 -/ EncryptedHWID::wBlobType
    //
    //    14 -\|
    //    00 -/ EncryptedHWID::wBlobLen
    //
    //    b9 30 59 3b 93 61 c9 f6 -\|
    //    b6 0b 1f dc 1a 85 67 39 -|
    //    dc 29 65 62             -/ EncryptedHWID::pBlob

    //    0x:8ed: MACData (0x10 bytes)
    //    42 a2 13 c7 54 ae b5 d5 -\|
    //    24 66 54 f3 1b af 8d fb -/ MACData

    struct ClientLicenseInfo_Send
    {
        ClientLicenseInfo_Send(OutStream & stream,
            uint8_t version, uint16_t license_size, uint8_t * license_data,
            uint8_t * hwid, uint8_t * signature)
        {
            uint16_t length = 16 + SEC_RANDOM_SIZE + SEC_MODULUS_SIZE + SEC_PADDING_SIZE +
                     license_size + LIC::LICENSE_HWID_SIZE + LIC::LICENSE_SIGNATURE_SIZE;

            uint8_t null_data[SEC_MODULUS_SIZE];
            memset(null_data, 0, sizeof(null_data));

            stream.out_uint8(LIC::LICENSE_INFO);
            stream.out_uint8(version); /* version */
            stream.out_uint16_le(length);

            // PreferredKeyExchangeAlg (4 bytes): A 32-bit unsigned integer that
            // indicates the key exchange algorithm chosen by the client. It MUST be set
            // to KEY_EXCHANGE_ALG_RSA (0x00000001), which indicates an RSA-based key
            // exchange with a 512-bit asymmetric key.<9>

            stream.out_uint32_le(LIC::KEY_EXCHANGE_ALG_RSA);

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

            stream.out_uint16_le(0);
            stream.out_uint16_le(0x0201);

            // ClientRandom (32 bytes): A 32-byte random number generated by the client
            // using a cryptographically secure pseudo-random number generator. The
            // ClientRandom and ServerRandom (see section 2.2.2.1) values, along with
            // the data in the EncryptedPreMasterSecret field, are used to generate
            // licensing encryption keys (see section 5.1.3). These keys are used to
            // encrypt licensing protocol messages (see sections 5.1.4 and 5.1.5).

            stream.out_copy_bytes(null_data, SEC_RANDOM_SIZE); // client_random

            // EncryptedPreMasterSecret (variable): A Licensing Binary BLOB structure
            // (see [MS-RDPBCGR] section 2.2.1.12.1.2) of type BB_RANDOM_BLOB (0x0002).
            // This BLOB contains an encrypted 48-byte random number. For instructions
            // on how to encrypt this random number, see section 5.1.2.1.

            // 2.2.1.12.1.2 Licensing Binary Blob (LICENSE_BINARY_BLOB)
            // --------------------------------------------------------
            // The LICENSE_BINARY_BLOB structure is used to encapsulate arbitrary
            // length binary licensing data.

            // wBlobType (2 bytes): A 16-bit, unsigned integer. The data type of
            // the binary information. If wBlobLen is set to 0, then the contents
            // of this field SHOULD be ignored.

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

            // wBlobLen (2 bytes): A 16-bit, unsigned integer. The size in bytes of the
            // binary information in the blobData field. If wBlobLen is set to 0, then the
            // blobData field is not include " in the Licensing Binary BLOB structure and the
            // contents of the wBlobType field SHOULD be ignored.

            // blobData (variable): Variable-length binary data. The size of this data in
            // bytes is given by the wBlobLen field. If wBlobLen is set to 0, then this field
            // is not include " in the Licensing Binary BLOB structure.

            // stream.out_uint16_le(LIC::BB_RANDOM_BLOB);
            stream.out_uint16_le(0);
            stream.out_uint16_le((SEC_MODULUS_SIZE + SEC_PADDING_SIZE));
            stream.out_copy_bytes(null_data, SEC_MODULUS_SIZE); // rsa_data
            stream.out_clear_bytes(SEC_PADDING_SIZE);

            stream.out_uint16_le(1);
            stream.out_uint16_le(license_size);
            stream.out_copy_bytes(license_data, license_size);
            stream.out_uint16_le(1);
            stream.out_uint16_le(LIC::LICENSE_HWID_SIZE);
            stream.out_copy_bytes(hwid, LIC::LICENSE_HWID_SIZE);
            stream.out_copy_bytes(signature, LIC::LICENSE_SIGNATURE_SIZE);
        }
    };

// The Client License Information packet is sent by a client that already has a license issued to it in response to
// the Server License Request (section 2.2.2.1) message.

    struct ClientLicenseInfo_Recv
    {
        uint8_t tag;
        uint8_t flags;
        uint16_t wMsgSize;

// PreferredKeyExchangeAlg (4 bytes): The content and format of this field are the same as the PreferredKeyExchangeAlg
//  field of the Client New License Request (section 2.2.2.2) message.
        uint32_t dwPreferredKeyExchangeAlg;

// PlatformId (4 bytes): The content and format of this field are the same as the PlatformId field of the Client New
//   License Request message.
        uint32_t dwPlatformId;

// ClientRandom (32 bytes): The content and format of this field are the same as the ClientRandom field of the Client
//   New License Request message.

        uint8_t client_random[32];

//  EncryptedPreMasterSecret (variable): The content and format of this field are the same as the
//    EncryptedPreMasterSecret field of the Client New License Request message.

//  LicenseInfo (variable): A Licensing Binary BLOB structure (see [MS-RDPBCGR] section 2.2.1.12.1.2) of type
//    BB_DATA_BLOB (0x0001). This BLOB contains the CAL (see the pbLicenseInfo field in section 2.2.2.6.1) that is
//    retrieved from the client's license store.

//  EncryptedHWID (variable): A Licensing Binary BLOB structure (see [MS-RDPBCGR] section 2.2.1.12.1.2). This BLOB
//    contains a Client Hardware Identification (section 2.2.2.3.1) structure encrypted with the licensing encryption
//    keys (see section 5.1.3), using RC4 (for instructions on how to perform the encryption, see section 5.1.4).

// MACData (16 bytes): An array of 16 bytes containing an MD5 digest (Message Authentication Code (MAC)) that is
//    generated over the unencrypted Client Hardware Identification structure. For instructions on how to generate this
//    message digest, see section 5.1.6; for a description of how the server uses the MACData field to verify the
//    integrity of the Client Hardware Identification structure, see section 3.1.5.1.

        uint16_t license_size;
        uint8_t * license_data;
        uint8_t * hwid;
        uint8_t * signature;

        explicit ClientLicenseInfo_Recv(InStream & stream) : license_size(0), license_data(nullptr),
        		hwid(nullptr), signature(nullptr)
        {
            /* tag(1) + flags(1) + wMsgSize(2) + dwPreferredKeyExchangeAlg(4) + dwPlatformId(4) +
             * client_random(SEC_RANDOM_SIZE) + wBlobType(2) + lenLicensingBlob(2)
             */
            const unsigned expected = 12 + SEC_RANDOM_SIZE + 4;
            if (!stream.in_check_rem(expected)){
                LOG(LOG_ERR, "Licence ClientLicenseInfo_Recv : Truncated data, need=%u, remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_LIC);
            }

            this->tag = stream.in_uint8();
            this->flags = stream.in_uint8();
            this->wMsgSize = stream.in_uint16_le();

            this->dwPreferredKeyExchangeAlg = stream.in_uint32_le();
            this->dwPlatformId = stream.in_uint32_le();
            stream.in_copy_bytes(this->client_random, SEC_RANDOM_SIZE); // client_random

            // EncryptedPreMasterSecret (variable): A Licensing Binary BLOB structure
            // (see [MS-RDPBCGR] section 2.2.1.12.1.2) of type BB_RANDOM_BLOB (0x0002).
            // This BLOB contains an encrypted 48-byte random number. For instructions
            // on how to encrypt this random number, see section 5.1.2.1.

            // 2.2.1.12.1.2 Licensing Binary Blob (LICENSE_BINARY_BLOB)
            // --------------------------------------------------------
            // The LICENSE_BINARY_BLOB structure is used to encapsulate arbitrary
            // length binary licensing data.

            // wBlobType (2 bytes): A 16-bit, unsigned integer. The data type of
            // the binary information. If wBlobLen is set to 0, then the contents
            // of this field SHOULD be ignored.

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

            // wBlobLen (2 bytes): A 16-bit, unsigned integer. The size in bytes of the
            // binary information in the blobData field. If wBlobLen is set to 0, then the
            // blobData field is not include " in the Licensing Binary BLOB structure and the
            // contents of the wBlobType field SHOULD be ignored.

            // blobData (variable): Variable-length binary data. The size of this data in
            // bytes is given by the wBlobLen field. If wBlobLen is set to 0, then this field
            // is not include " in the Licensing Binary BLOB structure.

            stream.in_skip_bytes(2); /* wBlobType */
            uint16_t lenLicensingBlob = stream.in_uint16_le();

            if (!stream.in_check_rem(lenLicensingBlob)){
                LOG(LOG_ERR, "Licence ClientLicenseInfo_Recv : Truncated blobData, need=%d, remains=%zu",
                    lenLicensingBlob, stream.in_remain());
                throw Error(ERR_LIC);
            }

            stream.in_skip_bytes(lenLicensingBlob); /* blobData */

//            stream.out_uint16_le(1);
//            stream.out_uint16_le(license_size);
//            stream.out_copy_bytes(license_data, license_size);
//            stream.out_uint16_le(1);
//            stream.out_uint16_le(LIC::LICENSE_HWID_SIZE);
//            stream.out_copy_bytes(hwid, LIC::LICENSE_HWID_SIZE);
//            stream.out_copy_bytes(signature, LIC::LICENSE_SIGNATURE_SIZE);

            // TODO Add missing fields
            //stream.end = stream.p;
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

        explicit PlatformChallenge_Recv(InStream & stream) {
            /* wMsgType(1) + bVersion(1) + wMsgSize(2) + dwConnectFlags(4) + wBlobType(2) + wBlobLen(2) +
             * blob(LICENSE_TOKEN_SIZE) + MACData(LICENSE_SIGNATURE_SIZE)
             */
            const unsigned expected = 12 + LICENSE_TOKEN_SIZE + LICENSE_SIGNATURE_SIZE;
            if (!stream.in_check_rem(expected)){
                LOG(LOG_ERR, "Licence PlatformChallenge_Recv : Truncated data, need=%u, remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_LIC);
            }

            this->wMsgType = stream.in_uint8();
            this->bVersion = stream.in_uint8();
            this->wMsgSize = stream.in_uint16_le();
            this->dwConnectFlags = stream.in_uint32_le(); // ignored
            this->encryptedPlatformChallenge.wBlobType = stream.in_uint16_le();      // ignored
            this->encryptedPlatformChallenge.wBlobLen = stream.in_uint16_le();
            if (this->encryptedPlatformChallenge.wBlobLen != LICENSE_TOKEN_SIZE) {
                LOG(LOG_ERR, "PlatformChallenge_Recv : token len = %u, expected %u",
                    this->encryptedPlatformChallenge.wBlobLen, LICENSE_TOKEN_SIZE);
                throw Error(ERR_LIC);
            }
            stream.in_copy_bytes(this->encryptedPlatformChallenge.blob, LICENSE_TOKEN_SIZE);
            stream.in_copy_bytes(this->MACData, LICENSE_SIGNATURE_SIZE);

            if (stream.in_remain()){
                LOG(LOG_ERR, "PlatformChallenge_Recv : unparsed data %zu", stream.in_remain());
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

    //    LicenseInfo::dwVersion (4 bytes): The content and format of this field are the same
    // as the dwVersion field of the Product Information (section 2.2.2.1.1) structure.
    // it's a 32-bit unsigned integer that contains the license version information. The high-order word
    // contains the major version of the operating system on which the terminal server is running, while
    // the low-order word contains the minor version.<6>

    //     LicenseInfo::cbScope (4 bytes): A 32-bit unsigned integer that contains the number of
    // bytes in the string contained in the pbScope field.

    //     LicenseInfo::pbScope (variable): Contains the nullptr-terminated ANSI character set string
    // giving the name of the issuer of this license. For example, for licenses issued
    // by TailSpin Toys, this field contains the string "TailSpin Toys".

    //     LicenseInfo::cbCompanyName (4 bytes): The content and format of this field are the same as
    // the cbCompanyName field of the Product Information structure.

    //     LicenseInfo::pbCompanyName (variable): The content and format of this field are the same as
    // the pbCompanyName field of the Product Information structure.

    //     LicenseInfo::cbProductId (4 bytes): The content and format of this field are the same as
    // the cbProductId field of the Product Information structure.

    //     LicenseInfo::pbProductId (variable): The content and format of this field are the same as
    // the pbProductId field of the Product Information structure.

    //     LicenseInfo::cbLicenseInfo (4 bytes): A 32-bit unsigned integer that contains the number
    // of bytes of binary data in the pbLicenseInfo field.

    //     LicenseInfo::pbLicenseInfo (variable): This field contains the CAL issued to the client by
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

    // clear text header
    //    03 03 07 08 09 00 ef 07
    // here begins encrypted data
    ///* 0000 */ de 18 8f 41 ad 37 5d 3a e7 6f 7d 15 38 1d fd 0b,  // ...A.7]:.o}.8...
    ///* 0010 */ b3 34 1c cc 9d cc c7 97 45 2b 00 32 4e 96 3c 9e,  // .4......E+.2N.<.
    ///* 0020 */ da cc 1a 07 1e 9f d6 c5 27 7a d1 cb 10 fc 61 19,  // ........'z....a.
    ///* 0030 */ e4 34 ea 2d cb 62 f1 c0 39 4f ea 05 d4 71 61 50,  // .4.-.b..9O...qaP
    ///* 0040 */ fd a0 ad 58 4a 48 ec a3 0a 39 3a 8a fd 1d 32 9e,  // ...XJH...9:...2.
    ///* 0050 */ c8 7d f1 16 f2 2c c4 1e a4 70 95 5d 93 c3 5d b3,  // .}...,...p.]..].
    ///* 0060 */ 55 15 7b 81 40 cc 48 7a d8 b2 bc bd 82 46 13 66,  // U.{.@.Hz.....F.f
    ///* 0070 */ 23 57 88 d3 66 a7 66 3b 04 cf f1 b6 c4 32 ee 41,  // #W..f.f;.....2.A
    ///* 0080 */ da 78 62 eb b1 97 ad eb 49 ac 46 b0 f6 3d c5 e7,  // .xb.....I.F..=..
    ///* 0090 */ 2e 51 14 fb 76 c9 ef 52 e5 6d 3e 36 05 db 97 c7,  // .Q..v..R.m>6....
    ///* 00a0 */ d9 1e f4 08 08 6c 1f bb 29 e8 24 36 85 43 32 91,  // .....l..).$6.C2.
    ///* 00b0 */ 0a 27 d8 ae 5d 35 73 0d 6a cb 59 a0 7e 9b 5b 8d,  // .'..]5s.j.Y.~.[.
    ///* 00c0 */ 00 60 a1 a2 b9 f8 49 d2 9f 5c 09 98 3e 1b a4 34,  // .`....I.....>..4
    ///* 00d0 */ d7 d3 29 38 95 85 86 2e c7 09 d5 34 47 1c d7 70,  // ..)8.......4G..p
    ///* 00e0 */ de 3b b9 07 11 aa 99 fd 0a d9 42 94 39 76 b9 ce,  // .;........B.9v..
    ///* 00f0 */ 50 f3 d5 61 47 9a 55 75 1d 32 75 0e a7 e1 8e 64,  // P..aG.Uu.2u....d
    ///* 0100 */ 38 6a ac 3c cd 59 e2 14 27 a8 11 10 c4 16 ca 0b,  // 8j.<.Y..'.......
    ///* 0110 */ d2 8a 7f 59 8e 64 37 2d df ac 17 c7 76 d2 76 42,  // ...Y.d7-....v.vB
    ///* 0120 */ 33 eb f5 9e 00 ce 02 30 0e 15 bf 58 32 11 38 2e,  // 3......0...X2.8.
    ///* 0130 */ 04 16 7a 8b b2 e0 93 89 7e 8e dc 74 06 fe a7 77,  // ..z.....~..t...w
    ///* 0140 */ 11 48 90 ae 67 a5 9d c1 99 32 3d 08 6e b5 fa 94,  // .H..g....2=.n...
    ///* 0150 */ d5 9b 85 2e ed 1e 8e 8e f7 6b 0d 1b b6 e4 b7 df,  // .........k......
    ///* 0160 */ 46 de 93 f7 ce 1e 91 ce 96 27 5a 55 93 bb f2 c1,  // F........'ZU....
    ///* 0170 */ af e3 90 81 08 5e c1 e9 12 23 ee 56 a3 55 31 00,  // .....^...#.V.U1.
    ///* 0180 */ e1 7c 54 a5 65 be db e0 45 c3 28 48 d7 cb 88 ae,  // .|T.e...E.(H....
    ///* 0190 */ 3d 91 3e c8 6c f0 4a ec 5a 3e 6f ac f6 cb de a7,  // =.>.l.J.Z>o.....
    ///* 01a0 */ 03 7e 5b 8d ab 94 75 12 fa 87 5e c2 d2 13 ad 5e,  // .~[...u...^....^
    ///* 01b0 */ cb a8 1a 71 b5 b5 62 cf f7 6a 10 81 d1 ba 57 8b,  // ...q..b..j....W.
    ///* 01c0 */ e1 a5 d1 99 fb 5c d4 2f 13 63 92 2e 83 49 12 ec,  // ......./.c...I..
    ///* 01d0 */ f7 1d 9f 19 d6 45 6b 6b 4c 0a a5 e7 ff e5 93 63,  // .....EkkL......c
    ///* 01e0 */ 63 33 14 48 7a 0d 8a e6 5c ab a6 54 bf 94 2b dc,  // c3.Hz......T..+.
    ///* 01f0 */ 1c 95 32 f2 07 fe 35 77 bf 41 27 18 5b 76 0d e9,  // ..2...5w.A'.[v..
    ///* 0200 */ 4e 69 93 11 ca 8f 53 7d aa f1 48 34 81 ff ae 43,  // Ni....S}..H4...C
    ///* 0210 */ ca 60 4e 01 95 e3 f1 e5 fe c1 f9 b7 24 38 5b cd,  // .`N.........$8[.
    ///* 0220 */ bb fb b5 ef 90 ff 63 8d 16 ca ec b2 55 5e a0 3e,  // ......c.....U^.>
    ///* 0230 */ 41 a5 61 89 d4 b1 57 2c 00 c6 18 44 68 2d fb 54,  // A.a...W,...Dh-.T
    ///* 0240 */ 68 aa 50 51 5f f0 40 64 36 c7 a9 99 91 95 31 39,  // h.PQ_.@d6.....19
    ///* 0250 */ 0d 25 a1 23 f9 ad a2 5d cf 34 29 db 97 fc d7 3a,  // .%.#...].4)....:
    ///* 0260 */ b7 16 df 41 19 15 17 3d 67 09 62 1c 82 ac 44 80,  // ...A...=g.b...D.
    ///* 0270 */ 37 d1 39 81 4e 56 ff 66 14 a5 b9 53 60 be 9c 1b,  // 7.9.NV.f...S`...
    ///* 0280 */ 01 b7 d8 3e 41 ff d1 8c 9f 3f 5a b2 05 95 2e 45,  // ...>A....?Z....E
    ///* 0290 */ 72 c6 a9 9b ce 31 84 c0 d9 e5 96 22 d3 34 28 2a,  // r....1.....".4(*
    ///* 02a0 */ 06 85 1f b2 5b f9 48 64 0c 73 71 b0 b7 cf 79 75,  // ....[.Hd.sq...yu
    ///* 02b0 */ 4b 89 09 7d ea 76 a7 72 0b 0c db 56 a6 e8 11 4c,  // K..}.v.r...V...L
    ///* 02c0 */ 3e a3 de c9 0a 87 37 24 2b 5a 26 95 d3 72 e4 43,  // >.....7$+Z&..r.C
    ///* 02d0 */ 20 1d c0 9b 85 69 f4 89 b6 4b eb 67 3d 9d 21 3e,  //  ....i...K.g=.!>
    ///* 02e0 */ 36 fa 0a fb c9 e7 b3 36 de 66 bc f7 01 23 7b 3f,  // 6......6.f...#{?
    ///* 02f0 */ a4 8c 68 a9 70 1d 6c d2 bb 53 97 1e b0 aa 9c 2d,  // ..h.p.l..S.....-
    ///* 0300 */ 3a b7 18 fe 01 c5 cf 67 ba 6d e1 06 8d 9d 8f 46,  // :......g.m.....F
    ///* 0310 */ 8c 17 53 5b 88 81 48 05 18 48 5e 8a f5 82 93 47,  // ..S[..H..H^....G
    ///* 0320 */ 5d 1c ba 83 79 68 07 c2 2f 05 a7 81 52 c1 40 73,  // ]...yh../...R.@s
    ///* 0330 */ a5 8d 1e 25 74 78 09 09 f9 4a 24 39 31 2f 48 b4,  // ...%tx...J$91/H.
    ///* 0340 */ 5d 9b 52 76 dc 6a b7 3b 58 0b 1d fe e3 f6 af 58,  // ].Rv.j.;X......X
    ///* 0350 */ 83 57 4b b3 55 31 bb 7e 1c 49 52 35 d1 ed bb 49,  // .WK.U1.~.IR5...I
    ///* 0360 */ 05 9e a8 d6 4c 70 7b 8c 05 95 ec f4 ce 06 55 78,  // ....Lp{.......Ux
    ///* 0370 */ fa 19 07 d9 93 0b f7 38 c9 21 16 54 5a 54 4d a4,  // .......8.!.TZTM.
    ///* 0380 */ ba d6 3e b9 e0 05 9d 95 43 5b 3b b2 7b c3 a8 df,  // ..>.....C[;.{...
    ///* 0390 */ 9d 09 db d1 ee 79 d7 56 52 49 a5 08 f6 8e 8a 4a,  // .....y.VRI.....J
    ///* 03a0 */ e1 77 a2 98 04 1a b0 a4 3e b5 44 dc ba bf cd 0c,  // .w......>.D.....
    ///* 03b0 */ 62 75 67 49 05 0d c8 73 53 d7 8d 7d 35 0d ea 8e,  // bugI...sS..}5...
    ///* 03c0 */ 09 81 41 66 d6 5f 45 06 45 07 eb e7 51 09 0e 2f,  // ..Af._E.E...Q../
    ///* 03d0 */ 55 7c 02 88 d3 8c 55 1a 27 73 5b b2 06 54 3f 81,  // U|....U.'s[..T?.
    ///* 03e0 */ f5 1f 3b 66 3f 88 d4 98 8a 55 9e 4b ff e8 22 c0,  // ..;f?....U.K..".
    ///* 03f0 */ 35 f4 6c 7b 5f 47 b2 bc 97 8c 89 bc bd f9 fc 65,  // 5.l{_G.........e
    ///* 0400 */ 9a c6 98 bf dd 04 8f 50 63 aa 16 84 ff 03 8b d6,  // .......Pc.......
    ///* 0410 */ b8 72 46 18 04 2b e4 cc 3d 2c 18 c6 9c 35 84 7d,  // .rF..+..=,...5.}
    ///* 0420 */ 8f a6 43 48 88 9f e7 f7 3b 1f 4a f5 e6 84 95 5e,  // ..CH....;.J....^
    ///* 0430 */ 40 5b f1 49 00 46 52 d3 18 8a 01 fd 98 d5 af 8c,  // @[.I.FR.........
    ///* 0440 */ 29 b8 59 d6 92 40 e7 52 eb 5c 8e da 45 ea 18 74,  // ).Y..@.R....E..t
    ///* 0450 */ a0 ed b5 ab 96 a0 77 5e 86 6e 2d e4 ab b7 d5 a1,  // ......w^.n-.....
    ///* 0460 */ 6a 73 92 81 f6 1c 24 22 e8 58 9d f4 c8 e6 3c 18,  // js....$".X....<.
    ///* 0470 */ 85 9f 60 e7 78 5a 03 ed f7 23 20 d9 41 6a 32 0c,  // ..`.xZ...# .Aj2.
    ///* 0480 */ c6 5d 95 18 68 79 b6 fd 94 7f 98 d2 40 a4 9a c2,  // .]..hy......@...
    ///* 0490 */ 25 2c 9e a5 ac b1 a5 5e 8c 62 d7 15 53 e1 f3 c7,  // %,.....^.b..S...
    ///* 04a0 */ 45 6a a1 81 71 75 66 17 8f 2a 1e cf cd d4 dd 0f,  // Ej..quf..*......
    ///* 04b0 */ 87 66 2d ad 18 5b 8e 41 f4 25 c0 18 22 58 99 fb,  // .f-..[.A.%.."X..
    ///* 04c0 */ 07 4a 73 93 ea 92 13 fd 79 ff 4d af 4f a4 c0 c6,  // .Js.....y.M.O...
    ///* 04d0 */ ea 7b 6b b8 a7 fe 7d 6f ea 9c 80 42 5c 6d 6e 1d,  // .{k...}o...B.mn.
    ///* 04e0 */ e8 e3 04 02 03 dc 54 65 06 ee c1 c8 f8 f4 37 6c,  // ......Te......7l
    ///* 04f0 */ 8b 35 82 20 0b 56 44 e6 15 8b d4 9b 35 fe 1f 0e,  // .5. .VD.....5...
    ///* 0500 */ 1d f2 a9 f5 c3 93 60 e6 21 0d 59 10 0f ea eb 5e,  // ......`.!.Y....^
    ///* 0510 */ 38 2d b9 8a f5 25 d3 91 11 d6 3a 33 2c f8 15 e3,  // 8-...%....:3,...
    ///* 0520 */ 4b eb 1c 89 4c 16 29 72 02 85 38 19 e3 6c 26 2f,  // K...L.)r..8..l&/
    ///* 0530 */ 6a b4 a4 0b a8 d3 d0 22 91 62 63 40 da 26 0a 33,  // j......".bc@.&.3
    ///* 0540 */ 8e a2 dc 34 90 05 78 ec cd 0e d7 7a 60 54 e1 c7,  // ...4..x....z`T..
    ///* 0550 */ 51 e2 6e ae 90 ae ed 00 a4 e0 a6 b2 1d 14 45 27,  // Q.n...........E'
    ///* 0560 */ b6 b2 14 36 f0 60 87 9e 2e 0f bf 42 8e c7 3d c7,  // ...6.`.....B..=.
    ///* 0570 */ af d1 e8 19 af e7 47 5c c2 cc f5 2a d1 9c 74 47,  // ......G....*..tG
    ///* 0580 */ ae 2c cf ca a2 ed 38 26 83 70 d6 dd 15 3e b1 71,  // .,....8&.p...>.q
    ///* 0590 */ 92 1b e5 5b c5 ae 0a a4 4f 3c 81 1b 00 36 02 0c,  // ...[....O<...6..
    ///* 05a0 */ c4 ce 45 3f 0a 15 df 72 06 02 de 69 8c e0 02 f0,  // ..E?...r...i....
    ///* 05b0 */ a4 06 21 43 f8 f7 b4 79 09 7b 43 46 1a e4 71 3d,  // ..!C...y.{CF..q=
    ///* 05c0 */ c3 e7 c2 d4 36 1e 61 cd 19 1b 7e 20 8a 92 ec 58,  // ....6.a...~ ...X
    ///* 05d0 */ aa b2 db b5 71 8f 65 52 09 57 98 50 2a 03 c5 be,  // ....q.eR.W.P*...
    ///* 05e0 */ e4 d1 58 40 b8 ba b0 0f d8 bb 6f c6 df 9a 4f 60,  // ..X@......o...O`
    ///* 05f0 */ be 28 28 7e b4 36 bc bb ed 20 6c 7d 8f cd d5 f5,  // .((~.6... l}....
    ///* 0600 */ 1e 5e 1e 6d 0a ba 59 89 75 42 59 4c ce 7a 4c e4,  // .^.m..Y.uBYL.zL.
    ///* 0610 */ 21 7e 32 06 8a 5e 9d 66 3c 7a 25 67 e2 f9 28 a8,  // !~2..^.f<z%g..(.
    ///* 0620 */ 71 ad 08 b3 e4 ff de de 8d 35 6b f4 67 fd 53 65,  // q........5k.g.Se
    ///* 0630 */ e2 36 ab 38 a8 51 37 9e b5 f7 0a de 17 22 cb 03,  // .6.8.Q7......"..
    ///* 0640 */ 15 f2 9d 2a 6d 42 5d 54 95 5b 92 91 d6 db 66 aa,  // ...*mB]T.[....f.
    ///* 0650 */ 78 6d 61 00 9e 8a b7 a7 23 7f d9 b0 a5 7d 03 83,  // xma.....#....}..
    ///* 0660 */ af 26 8b bb 3d 40 be 14 08 dc 48 c2 dc 03 20 e0,  // .&..=@....H... .
    ///* 0670 */ 95 c0 ad 67 79 ed 09 c6 c3 a2 7d 3b 08 82 1c 15,  // ...gy.....};....
    ///* 0680 */ 5c ae 1d 1f e3 64 e3 9e ab 14 56 c9 9f fb c8 b8,  // .....d....V.....
    ///* 0690 */ f4 31 0d d7 92 80 8f db 7e 78 d4 d9 79 c0 52 a8,  // .1......~x..y.R.
    ///* 06a0 */ 9f 8c 48 a1 88 ed 78 c8 c6 76 4a 4f a5 a5 a6 ac,  // ..H...x..vJO....
    ///* 06b0 */ 7d 3f 41 88 ae df fa db c0 dc 61 f9 56 52 ac f6,  // }?A.......a.VR..
    ///* 06c0 */ c9 25 03 74 d7 2c 22 ee a9 b6 27 66 8b 12 09 4e,  // .%.t.,"...'f...N
    ///* 06d0 */ 6e 7a 9f be 4e e4 af c1 da 6b 04 c5 dd 79 10 32,  // nz..N....k...y.2
    ///* 06e0 */ cb bd 12 7b d9 3c 92 3b 2a 7d 52 f7 d6 73 5a 06,  // ...{.<.;*}R..sZ.
    ///* 06f0 */ d1 f9 5f a9 69 8c 04 23 b8 e6 2e ce 0d ea 07 30,  // .._.i..#.......0
    ///* 0700 */ fd 00 3a 0b 61 dd b1 b4 96 9a a9 0e a6 ce ee 98,  // ..:.a...........
    ///* 0710 */ 1c d5 58 40 1b 34 4a 56 b3 f6 a7 bc 97 e9 85 9b,  // ..X@.4JV........
    ///* 0720 */ 10 43 df a6 35 b4 0e 41 a5 ae 56 28 2c cc 10 1b,  // .C..5..A..V(,...
    ///* 0730 */ 71 36 60 1f fc 4b c3 4a 1e b5 51 d9 de b7 5b 9f,  // q6`..K.J..Q...[.
    ///* 0740 */ 73 b2 a6 6d a0 df ad c6 be 73 a5 4f 90 13 08 b3,  // s..m.....s.O....
    ///* 0750 */ cf eb 5e 7e 5c 6e 99 5a a0 36 75 03 82 20 a5 74,  // ..^~.n.Z.6u.. .t
    ///* 0760 */ 8c 1e e4 4b cb 77 97 4d 33 55 fb 2d cb 09 d2 57,  // ...K.w.M3U.-...W
    ///* 0770 */ dd 69 e9 37 55 c6 f5 0c 54 02 c4 7c e7 9f c6 1f,  // .i.7U...T..|....
    ///* 0780 */ 9d 20 2d 4a 7a 47 b2 8f 4e 17 88 1e a1 29 ee 26,  // . -JzG..N....).&
    ///* 0790 */ 79 53 2a 2f 8b 38 de 29 f2 a4 cd 39 29 c2 0a 01,  // yS*/.8.)...9)...
    ///* 07a0 */ 7f b0 5e 79 84 8d b7 9c 2e 15 40 35 17 d0 ea 7e,  // ..^y......@5...~
    ///* 07b0 */ 8e a4 ba f9 06 a1 fc a8 dc 74 db 99 5e da bd ff,  // .........t..^...
    ///* 07c0 */ 43 b9 d7 5c c0 62 be ae 16 be 60 7c 2c 8f bd 53,  // C....b....`|,..S
    ///* 07d0 */ 42 8c 98 57 73 de a0 94 6f 7a d1 30 ce 26 e2 bf,  // B..Ws...oz.0.&..
    ///* 07e0 */ de 9f 6f 6c 8f 8c 0c 89 2f 7d ce 1f 2f af 37,     // ..ol..../}../.7
    // MAC Data
    //   ed e8 bf d6 13 a0 f5 80 4a e5 ff 85 16 fa cb 1f

    // --------------------------------------------------------------------------------
    //    0x00: LICENSE_PREAMBLE    (4 bytes)
    //    03 -> LICENSE_PREAMBLE::bMsgType = SERVER_NEW_LICENSE
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
    ///* 0000 */ de 18 8f 41 ad 37 5d 3a e7 6f 7d 15 38 1d fd 0b,  // ...A.7]:.o}.8...
    ///* 0010 */ b3 34 1c cc 9d cc c7 97 45 2b 00 32 4e 96 3c 9e,  // .4......E+.2N.<.
    ///* 0020 */ da cc 1a 07 1e 9f d6 c5 27 7a d1 cb 10 fc 61 19,  // ........'z....a.
    ///* 0030 */ e4 34 ea 2d cb 62 f1 c0 39 4f ea 05 d4 71 61 50,  // .4.-.b..9O...qaP
    ///* 0040 */ fd a0 ad 58 4a 48 ec a3 0a 39 3a 8a fd 1d 32 9e,  // ...XJH...9:...2.
    ///* 0050 */ c8 7d f1 16 f2 2c c4 1e a4 70 95 5d 93 c3 5d b3,  // .}...,...p.]..].
    ///* 0060 */ 55 15 7b 81 40 cc 48 7a d8 b2 bc bd 82 46 13 66,  // U.{.@.Hz.....F.f
    ///* 0070 */ 23 57 88 d3 66 a7 66 3b 04 cf f1 b6 c4 32 ee 41,  // #W..f.f;.....2.A
    ///* 0080 */ da 78 62 eb b1 97 ad eb 49 ac 46 b0 f6 3d c5 e7,  // .xb.....I.F..=..
    ///* 0090 */ 2e 51 14 fb 76 c9 ef 52 e5 6d 3e 36 05 db 97 c7,  // .Q..v..R.m>6....
    ///* 00a0 */ d9 1e f4 08 08 6c 1f bb 29 e8 24 36 85 43 32 91,  // .....l..).$6.C2.
    ///* 00b0 */ 0a 27 d8 ae 5d 35 73 0d 6a cb 59 a0 7e 9b 5b 8d,  // .'..]5s.j.Y.~.[.
    ///* 00c0 */ 00 60 a1 a2 b9 f8 49 d2 9f 5c 09 98 3e 1b a4 34,  // .`....I.....>..4
    ///* 00d0 */ d7 d3 29 38 95 85 86 2e c7 09 d5 34 47 1c d7 70,  // ..)8.......4G..p
    ///* 00e0 */ de 3b b9 07 11 aa 99 fd 0a d9 42 94 39 76 b9 ce,  // .;........B.9v..
    ///* 00f0 */ 50 f3 d5 61 47 9a 55 75 1d 32 75 0e a7 e1 8e 64,  // P..aG.Uu.2u....d
    ///* 0100 */ 38 6a ac 3c cd 59 e2 14 27 a8 11 10 c4 16 ca 0b,  // 8j.<.Y..'.......
    ///* 0110 */ d2 8a 7f 59 8e 64 37 2d df ac 17 c7 76 d2 76 42,  // ...Y.d7-....v.vB
    ///* 0120 */ 33 eb f5 9e 00 ce 02 30 0e 15 bf 58 32 11 38 2e,  // 3......0...X2.8.
    ///* 0130 */ 04 16 7a 8b b2 e0 93 89 7e 8e dc 74 06 fe a7 77,  // ..z.....~..t...w
    ///* 0140 */ 11 48 90 ae 67 a5 9d c1 99 32 3d 08 6e b5 fa 94,  // .H..g....2=.n...
    ///* 0150 */ d5 9b 85 2e ed 1e 8e 8e f7 6b 0d 1b b6 e4 b7 df,  // .........k......
    ///* 0160 */ 46 de 93 f7 ce 1e 91 ce 96 27 5a 55 93 bb f2 c1,  // F........'ZU....
    ///* 0170 */ af e3 90 81 08 5e c1 e9 12 23 ee 56 a3 55 31 00,  // .....^...#.V.U1.
    ///* 0180 */ e1 7c 54 a5 65 be db e0 45 c3 28 48 d7 cb 88 ae,  // .|T.e...E.(H....
    ///* 0190 */ 3d 91 3e c8 6c f0 4a ec 5a 3e 6f ac f6 cb de a7,  // =.>.l.J.Z>o.....
    ///* 01a0 */ 03 7e 5b 8d ab 94 75 12 fa 87 5e c2 d2 13 ad 5e,  // .~[...u...^....^
    ///* 01b0 */ cb a8 1a 71 b5 b5 62 cf f7 6a 10 81 d1 ba 57 8b,  // ...q..b..j....W.
    ///* 01c0 */ e1 a5 d1 99 fb 5c d4 2f 13 63 92 2e 83 49 12 ec,  // ......./.c...I..
    ///* 01d0 */ f7 1d 9f 19 d6 45 6b 6b 4c 0a a5 e7 ff e5 93 63,  // .....EkkL......c
    ///* 01e0 */ 63 33 14 48 7a 0d 8a e6 5c ab a6 54 bf 94 2b dc,  // c3.Hz......T..+.
    ///* 01f0 */ 1c 95 32 f2 07 fe 35 77 bf 41 27 18 5b 76 0d e9,  // ..2...5w.A'.[v..
    ///* 0200 */ 4e 69 93 11 ca 8f 53 7d aa f1 48 34 81 ff ae 43,  // Ni....S}..H4...C
    ///* 0210 */ ca 60 4e 01 95 e3 f1 e5 fe c1 f9 b7 24 38 5b cd,  // .`N.........$8[.
    ///* 0220 */ bb fb b5 ef 90 ff 63 8d 16 ca ec b2 55 5e a0 3e,  // ......c.....U^.>
    ///* 0230 */ 41 a5 61 89 d4 b1 57 2c 00 c6 18 44 68 2d fb 54,  // A.a...W,...Dh-.T
    ///* 0240 */ 68 aa 50 51 5f f0 40 64 36 c7 a9 99 91 95 31 39,  // h.PQ_.@d6.....19
    ///* 0250 */ 0d 25 a1 23 f9 ad a2 5d cf 34 29 db 97 fc d7 3a,  // .%.#...].4)....:
    ///* 0260 */ b7 16 df 41 19 15 17 3d 67 09 62 1c 82 ac 44 80,  // ...A...=g.b...D.
    ///* 0270 */ 37 d1 39 81 4e 56 ff 66 14 a5 b9 53 60 be 9c 1b,  // 7.9.NV.f...S`...
    ///* 0280 */ 01 b7 d8 3e 41 ff d1 8c 9f 3f 5a b2 05 95 2e 45,  // ...>A....?Z....E
    ///* 0290 */ 72 c6 a9 9b ce 31 84 c0 d9 e5 96 22 d3 34 28 2a,  // r....1.....".4(*
    ///* 02a0 */ 06 85 1f b2 5b f9 48 64 0c 73 71 b0 b7 cf 79 75,  // ....[.Hd.sq...yu
    ///* 02b0 */ 4b 89 09 7d ea 76 a7 72 0b 0c db 56 a6 e8 11 4c,  // K..}.v.r...V...L
    ///* 02c0 */ 3e a3 de c9 0a 87 37 24 2b 5a 26 95 d3 72 e4 43,  // >.....7$+Z&..r.C
    ///* 02d0 */ 20 1d c0 9b 85 69 f4 89 b6 4b eb 67 3d 9d 21 3e,  //  ....i...K.g=.!>
    ///* 02e0 */ 36 fa 0a fb c9 e7 b3 36 de 66 bc f7 01 23 7b 3f,  // 6......6.f...#{?
    ///* 02f0 */ a4 8c 68 a9 70 1d 6c d2 bb 53 97 1e b0 aa 9c 2d,  // ..h.p.l..S.....-
    ///* 0300 */ 3a b7 18 fe 01 c5 cf 67 ba 6d e1 06 8d 9d 8f 46,  // :......g.m.....F
    ///* 0310 */ 8c 17 53 5b 88 81 48 05 18 48 5e 8a f5 82 93 47,  // ..S[..H..H^....G
    ///* 0320 */ 5d 1c ba 83 79 68 07 c2 2f 05 a7 81 52 c1 40 73,  // ]...yh../...R.@s
    ///* 0330 */ a5 8d 1e 25 74 78 09 09 f9 4a 24 39 31 2f 48 b4,  // ...%tx...J$91/H.
    ///* 0340 */ 5d 9b 52 76 dc 6a b7 3b 58 0b 1d fe e3 f6 af 58,  // ].Rv.j.;X......X
    ///* 0350 */ 83 57 4b b3 55 31 bb 7e 1c 49 52 35 d1 ed bb 49,  // .WK.U1.~.IR5...I
    ///* 0360 */ 05 9e a8 d6 4c 70 7b 8c 05 95 ec f4 ce 06 55 78,  // ....Lp{.......Ux
    ///* 0370 */ fa 19 07 d9 93 0b f7 38 c9 21 16 54 5a 54 4d a4,  // .......8.!.TZTM.
    ///* 0380 */ ba d6 3e b9 e0 05 9d 95 43 5b 3b b2 7b c3 a8 df,  // ..>.....C[;.{...
    ///* 0390 */ 9d 09 db d1 ee 79 d7 56 52 49 a5 08 f6 8e 8a 4a,  // .....y.VRI.....J
    ///* 03a0 */ e1 77 a2 98 04 1a b0 a4 3e b5 44 dc ba bf cd 0c,  // .w......>.D.....
    ///* 03b0 */ 62 75 67 49 05 0d c8 73 53 d7 8d 7d 35 0d ea 8e,  // bugI...sS..}5...
    ///* 03c0 */ 09 81 41 66 d6 5f 45 06 45 07 eb e7 51 09 0e 2f,  // ..Af._E.E...Q../
    ///* 03d0 */ 55 7c 02 88 d3 8c 55 1a 27 73 5b b2 06 54 3f 81,  // U|....U.'s[..T?.
    ///* 03e0 */ f5 1f 3b 66 3f 88 d4 98 8a 55 9e 4b ff e8 22 c0,  // ..;f?....U.K..".
    ///* 03f0 */ 35 f4 6c 7b 5f 47 b2 bc 97 8c 89 bc bd f9 fc 65,  // 5.l{_G.........e
    ///* 0400 */ 9a c6 98 bf dd 04 8f 50 63 aa 16 84 ff 03 8b d6,  // .......Pc.......
    ///* 0410 */ b8 72 46 18 04 2b e4 cc 3d 2c 18 c6 9c 35 84 7d,  // .rF..+..=,...5.}
    ///* 0420 */ 8f a6 43 48 88 9f e7 f7 3b 1f 4a f5 e6 84 95 5e,  // ..CH....;.J....^
    ///* 0430 */ 40 5b f1 49 00 46 52 d3 18 8a 01 fd 98 d5 af 8c,  // @[.I.FR.........
    ///* 0440 */ 29 b8 59 d6 92 40 e7 52 eb 5c 8e da 45 ea 18 74,  // ).Y..@.R....E..t
    ///* 0450 */ a0 ed b5 ab 96 a0 77 5e 86 6e 2d e4 ab b7 d5 a1,  // ......w^.n-.....
    ///* 0460 */ 6a 73 92 81 f6 1c 24 22 e8 58 9d f4 c8 e6 3c 18,  // js....$".X....<.
    ///* 0470 */ 85 9f 60 e7 78 5a 03 ed f7 23 20 d9 41 6a 32 0c,  // ..`.xZ...# .Aj2.
    ///* 0480 */ c6 5d 95 18 68 79 b6 fd 94 7f 98 d2 40 a4 9a c2,  // .]..hy......@...
    ///* 0490 */ 25 2c 9e a5 ac b1 a5 5e 8c 62 d7 15 53 e1 f3 c7,  // %,.....^.b..S...
    ///* 04a0 */ 45 6a a1 81 71 75 66 17 8f 2a 1e cf cd d4 dd 0f,  // Ej..quf..*......
    ///* 04b0 */ 87 66 2d ad 18 5b 8e 41 f4 25 c0 18 22 58 99 fb,  // .f-..[.A.%.."X..
    ///* 04c0 */ 07 4a 73 93 ea 92 13 fd 79 ff 4d af 4f a4 c0 c6,  // .Js.....y.M.O...
    ///* 04d0 */ ea 7b 6b b8 a7 fe 7d 6f ea 9c 80 42 5c 6d 6e 1d,  // .{k...}o...B.mn.
    ///* 04e0 */ e8 e3 04 02 03 dc 54 65 06 ee c1 c8 f8 f4 37 6c,  // ......Te......7l
    ///* 04f0 */ 8b 35 82 20 0b 56 44 e6 15 8b d4 9b 35 fe 1f 0e,  // .5. .VD.....5...
    ///* 0500 */ 1d f2 a9 f5 c3 93 60 e6 21 0d 59 10 0f ea eb 5e,  // ......`.!.Y....^
    ///* 0510 */ 38 2d b9 8a f5 25 d3 91 11 d6 3a 33 2c f8 15 e3,  // 8-...%....:3,...
    ///* 0520 */ 4b eb 1c 89 4c 16 29 72 02 85 38 19 e3 6c 26 2f,  // K...L.)r..8..l&/
    ///* 0530 */ 6a b4 a4 0b a8 d3 d0 22 91 62 63 40 da 26 0a 33,  // j......".bc@.&.3
    ///* 0540 */ 8e a2 dc 34 90 05 78 ec cd 0e d7 7a 60 54 e1 c7,  // ...4..x....z`T..
    ///* 0550 */ 51 e2 6e ae 90 ae ed 00 a4 e0 a6 b2 1d 14 45 27,  // Q.n...........E'
    ///* 0560 */ b6 b2 14 36 f0 60 87 9e 2e 0f bf 42 8e c7 3d c7,  // ...6.`.....B..=.
    ///* 0570 */ af d1 e8 19 af e7 47 5c c2 cc f5 2a d1 9c 74 47,  // ......G....*..tG
    ///* 0580 */ ae 2c cf ca a2 ed 38 26 83 70 d6 dd 15 3e b1 71,  // .,....8&.p...>.q
    ///* 0590 */ 92 1b e5 5b c5 ae 0a a4 4f 3c 81 1b 00 36 02 0c,  // ...[....O<...6..
    ///* 05a0 */ c4 ce 45 3f 0a 15 df 72 06 02 de 69 8c e0 02 f0,  // ..E?...r...i....
    ///* 05b0 */ a4 06 21 43 f8 f7 b4 79 09 7b 43 46 1a e4 71 3d,  // ..!C...y.{CF..q=
    ///* 05c0 */ c3 e7 c2 d4 36 1e 61 cd 19 1b 7e 20 8a 92 ec 58,  // ....6.a...~ ...X
    ///* 05d0 */ aa b2 db b5 71 8f 65 52 09 57 98 50 2a 03 c5 be,  // ....q.eR.W.P*...
    ///* 05e0 */ e4 d1 58 40 b8 ba b0 0f d8 bb 6f c6 df 9a 4f 60,  // ..X@......o...O`
    ///* 05f0 */ be 28 28 7e b4 36 bc bb ed 20 6c 7d 8f cd d5 f5,  // .((~.6... l}....
    ///* 0600 */ 1e 5e 1e 6d 0a ba 59 89 75 42 59 4c ce 7a 4c e4,  // .^.m..Y.uBYL.zL.
    ///* 0610 */ 21 7e 32 06 8a 5e 9d 66 3c 7a 25 67 e2 f9 28 a8,  // !~2..^.f<z%g..(.
    ///* 0620 */ 71 ad 08 b3 e4 ff de de 8d 35 6b f4 67 fd 53 65,  // q........5k.g.Se
    ///* 0630 */ e2 36 ab 38 a8 51 37 9e b5 f7 0a de 17 22 cb 03,  // .6.8.Q7......"..
    ///* 0640 */ 15 f2 9d 2a 6d 42 5d 54 95 5b 92 91 d6 db 66 aa,  // ...*mB]T.[....f.
    ///* 0650 */ 78 6d 61 00 9e 8a b7 a7 23 7f d9 b0 a5 7d 03 83,  // xma.....#....}..
    ///* 0660 */ af 26 8b bb 3d 40 be 14 08 dc 48 c2 dc 03 20 e0,  // .&..=@....H... .
    ///* 0670 */ 95 c0 ad 67 79 ed 09 c6 c3 a2 7d 3b 08 82 1c 15,  // ...gy.....};....
    ///* 0680 */ 5c ae 1d 1f e3 64 e3 9e ab 14 56 c9 9f fb c8 b8,  // .....d....V.....
    ///* 0690 */ f4 31 0d d7 92 80 8f db 7e 78 d4 d9 79 c0 52 a8,  // .1......~x..y.R.
    ///* 06a0 */ 9f 8c 48 a1 88 ed 78 c8 c6 76 4a 4f a5 a5 a6 ac,  // ..H...x..vJO....
    ///* 06b0 */ 7d 3f 41 88 ae df fa db c0 dc 61 f9 56 52 ac f6,  // }?A.......a.VR..
    ///* 06c0 */ c9 25 03 74 d7 2c 22 ee a9 b6 27 66 8b 12 09 4e,  // .%.t.,"...'f...N
    ///* 06d0 */ 6e 7a 9f be 4e e4 af c1 da 6b 04 c5 dd 79 10 32,  // nz..N....k...y.2
    ///* 06e0 */ cb bd 12 7b d9 3c 92 3b 2a 7d 52 f7 d6 73 5a 06,  // ...{.<.;*}R..sZ.
    ///* 06f0 */ d1 f9 5f a9 69 8c 04 23 b8 e6 2e ce 0d ea 07 30,  // .._.i..#.......0
    ///* 0700 */ fd 00 3a 0b 61 dd b1 b4 96 9a a9 0e a6 ce ee 98,  // ..:.a...........
    ///* 0710 */ 1c d5 58 40 1b 34 4a 56 b3 f6 a7 bc 97 e9 85 9b,  // ..X@.4JV........
    ///* 0720 */ 10 43 df a6 35 b4 0e 41 a5 ae 56 28 2c cc 10 1b,  // .C..5..A..V(,...
    ///* 0730 */ 71 36 60 1f fc 4b c3 4a 1e b5 51 d9 de b7 5b 9f,  // q6`..K.J..Q...[.
    ///* 0740 */ 73 b2 a6 6d a0 df ad c6 be 73 a5 4f 90 13 08 b3,  // s..m.....s.O....
    ///* 0750 */ cf eb 5e 7e 5c 6e 99 5a a0 36 75 03 82 20 a5 74,  // ..^~.n.Z.6u.. .t
    ///* 0760 */ 8c 1e e4 4b cb 77 97 4d 33 55 fb 2d cb 09 d2 57,  // ...K.w.M3U.-...W
    ///* 0770 */ dd 69 e9 37 55 c6 f5 0c 54 02 c4 7c e7 9f c6 1f,  // .i.7U...T..|....
    ///* 0780 */ 9d 20 2d 4a 7a 47 b2 8f 4e 17 88 1e a1 29 ee 26,  // . -JzG..N....).&
    ///* 0790 */ 79 53 2a 2f 8b 38 de 29 f2 a4 cd 39 29 c2 0a 01,  // yS*/.8.)...9)...
    ///* 07a0 */ 7f b0 5e 79 84 8d b7 9c 2e 15 40 35 17 d0 ea 7e,  // ..^y......@5...~
    ///* 07b0 */ 8e a4 ba f9 06 a1 fc a8 dc 74 db 99 5e da bd ff,  // .........t..^...
    ///* 07c0 */ 43 b9 d7 5c c0 62 be ae 16 be 60 7c 2c 8f bd 53,  // C....b....`|,..S
    ///* 07d0 */ 42 8c 98 57 73 de a0 94 6f 7a d1 30 ce 26 e2 bf,  // B..Ws...oz.0.&..
    ///* 07e0 */ de 9f 6f 6c 8f 8c 0c 89 2f 7d ce 1f 2f af 37,     // ..ol..../}../.7
    //    EncryptedLicenseInfo::pBlob

    //   (for tests purpose the above data should be decrypted
    //   with RC4 key = 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0)

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

            //     LicenseInfo::pbScope (variable): Contains the nullptr-terminated ANSI character set string
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

        NewLicense_Recv(InStream & stream, uint8_t (&license_key)[16]) {
            /* wMsgType(1) + bVersion(1) + wMsgSize(2) + wBlobType(2) + wBlobLen(2)
             */
            unsigned expected = 8;
            if (!stream.in_check_rem(expected)){
                LOG(LOG_ERR, "Licence NewLicense_Recv : Truncated data, need=%u, remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_LIC);
            }

            this->wMsgType = stream.in_uint8();
            this->bVersion = stream.in_uint8();
            this->wMsgSize = stream.in_uint16_le();

            this->licenseInfo.wBlobType = stream.in_uint16_le();
            this->licenseInfo.wBlobLen = stream.in_uint16_le();

            // following data is encrypted using license_key
            SslRC4 rc4;
            rc4.set_key(make_array_view(license_key));

            if (!stream.in_check_rem(this->licenseInfo.wBlobLen)){
                LOG(LOG_ERR, "Licence NewLicense_Recv : Truncated license data, need=%u, remains=%zu",
                    this->licenseInfo.wBlobLen, stream.in_remain());
                throw Error(ERR_LIC);
            }

            uint8_t * data = const_cast<uint8_t*>(stream.get_current());
            // size, in, out
            rc4.crypt(this->licenseInfo.wBlobLen, data, data);

            expected = 8; /* dwVersion(4) + cbScope(4) */
            if (!stream.in_check_rem(expected)){
                LOG(LOG_ERR, "Licence NewLicense_Recv : Truncated unencrypted license data, need=%u, remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_LIC);
            }

            // now it's unencrypted, we can read it
            this->licenseInfo.dwVersion = stream.in_uint32_le();
            this->licenseInfo.cbScope = stream.in_uint32_le();

            if (!stream.in_check_rem(this->licenseInfo.cbScope)){
                LOG(LOG_ERR, "Licence NewLicense_Recv : Truncated license info pbScope, need=%u, remains=%zu",
                    this->licenseInfo.cbScope, stream.in_remain());
                throw Error(ERR_LIC);
            }
            stream.in_copy_bytes(this->licenseInfo.pbScope, this->licenseInfo.cbScope);

            if (!stream.in_check_rem(4)){
                LOG(LOG_ERR, "Licence NewLicense_Recv : Truncated license info cbCompanyName, need=4, remains=%zu",
                    stream.in_remain());
                throw Error(ERR_LIC);
            }
            this->licenseInfo.cbCompanyName = stream.in_uint32_le();
            if (!stream.in_check_rem(this->licenseInfo.cbCompanyName)){
                LOG(LOG_ERR, "Licence NewLicense_Recv : Truncated license info pbCompanyName, need=%u, remains=%zu",
                    this->licenseInfo.cbCompanyName, stream.in_remain());
                throw Error(ERR_LIC);
            }
            stream.in_copy_bytes(this->licenseInfo.pbCompanyName, this->licenseInfo.cbCompanyName);

            if (!stream.in_check_rem(4)){
                LOG(LOG_ERR, "Licence NewLicense_Recv : Truncated license info cbProductId, need=4, remains=%zu",
                    stream.in_remain());
                throw Error(ERR_LIC);
            }
            this->licenseInfo.cbProductId = stream.in_uint32_le();
            if (!stream.in_check_rem(this->licenseInfo.cbProductId)){
                LOG(LOG_ERR, "Licence NewLicense_Recv : Truncated license info pbProductId, need=%u, remains=%zu",
                    this->licenseInfo.cbProductId, stream.in_remain());
                throw Error(ERR_LIC);
            }
            stream.in_copy_bytes(this->licenseInfo.pbProductId, this->licenseInfo.cbProductId);

            if (!stream.in_check_rem(4)){
                LOG(LOG_ERR, "Licence NewLicense_Recv : Truncated license info cbLicenseInfo, need=4, remains=%zu",
                    stream.in_remain());
                throw Error(ERR_LIC);
            }
            this->licenseInfo.cbLicenseInfo = stream.in_uint32_le();
            if (!stream.in_check_rem(this->licenseInfo.cbLicenseInfo)){
                LOG(LOG_ERR, "Licence NewLicense_Recv : Truncated license info pbLicenseInfo, need=%u, remains=%zu",
                    this->licenseInfo.cbLicenseInfo, stream.in_remain());
                throw Error(ERR_LIC);
            }
            stream.in_copy_bytes(this->licenseInfo.pbLicenseInfo, this->licenseInfo.cbLicenseInfo);

            if (!stream.in_check_rem(LICENSE_SIGNATURE_SIZE)){
                LOG(LOG_ERR, "Licence NewLicense_Recv : Truncated license info MACData, need=%u, remains=%zu",
                    LICENSE_SIGNATURE_SIZE, stream.in_remain());
                throw Error(ERR_LIC);
            }
            stream.in_copy_bytes(this->MACData, LICENSE_SIGNATURE_SIZE);

            if (stream.in_remain()){
                LOG(LOG_ERR, "NewLicense_Recv : unparsed data %zu", stream.in_remain());
                throw Error(ERR_LIC);
            }

        }
    };

//    4.7 SERVER UPGRADE LICENSE
//    --------------------------

//    The Server Upgrade License message is sent to the client to upgrade a license in its license store.
//    The message type is UPGRADE_LICENSE (0x04) in the licensing preamble. See section 2.2.2.7 for more information.

//    Basically it's the same message format as NEW LICENSE

//    0x00: LICENSE_PREAMBLE (4 bytes)
//       04 -> LICENSE_PREAMBLE::bMsgType = SERVER_UPGRADE_LICENSE
//
//    03 -> LICENSE_PREAMBLE::bVersion = 3 (RPD 5.0, 5.2, 6.0)
//
//    95 -\|
//    1b -/ LICENSE_PREAMBLE::wMsgSize = 0x1b95 bytes

//    0x04: EncryptedLicenseInfo (2 + 2 + 0x1b7d = 0x1b81 bytes)
//    09 -\|
//    00 -/ EncryptedLicenseInfo::wBlobType = BB_ENCRYPTED_DATA_BLOB
//
//    7d -\|
//    1b -/ EncryptedLicenseInfo::wBlobLen = 0x1b7d bytes
//
//    The remaining part of this blob is the EncryptedLicenseInfo. The decrypted
//            LicenseInfo blob and data fields can be seen in section 4.1.6: Protocol
//            Examples, SERVER_NEW_LICENSE.
//    0x1b85: MACData
//    73 da-36 1e 92 c8 d0 78 12 c3 1c d3 68 a5 c6 00 -> MACData

    struct UpgradeLicense_Recv
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

            //     LicenseInfo::pbScope (variable): Contains the nullptr-terminated ANSI character set string
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

        UpgradeLicense_Recv(InStream & stream, uint8_t (&license_key)[16]){
            /* wMsgType(1) + bVersion(1) + wMsgSize(2) + wBlobType(2) + wBlobLen(2)
             */
            unsigned expected = 8;
            if (!stream.in_check_rem(expected)){
                LOG(LOG_ERR, "Licence UpgradeLicense_Recv : Truncated data, need=%u, remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_LIC);
            }

            this->wMsgType = stream.in_uint8();
            this->bVersion = stream.in_uint8();
            this->wMsgSize = stream.in_uint16_le();

            this->licenseInfo.wBlobType = stream.in_uint16_le();
            this->licenseInfo.wBlobLen = stream.in_uint16_le();

            // following data is encrypted using license_key
            SslRC4 rc4;
            rc4.set_key(make_array_view(license_key));

            uint8_t * data = const_cast<uint8_t*>(stream.get_current());
            // size, in, out
            rc4.crypt(this->licenseInfo.wBlobLen, data, data);

            expected = 8; /* dwVersion(4) + cbScope(4) */
            if (!stream.in_check_rem(expected)){
                LOG(LOG_ERR, "Licence UpgradeLicense_Recv : Truncated unencrypted license data, need=%u, remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_LIC);
            }

            this->licenseInfo.dwVersion = stream.in_uint32_le();
            this->licenseInfo.cbScope = stream.in_uint32_le();

            if (!stream.in_check_rem(this->licenseInfo.cbScope)){
                LOG(LOG_ERR, "Licence UpgradeLicense_Recv : Truncated license info pbScope, need=%u, remains=%zu",
                    this->licenseInfo.cbScope, stream.in_remain());
                throw Error(ERR_LIC);
            }
            stream.in_copy_bytes(this->licenseInfo.pbScope, this->licenseInfo.cbScope);

            if (!stream.in_check_rem(4)){
                LOG(LOG_ERR, "Licence UpgradeLicense_Recv : Truncated license info cbCompanyName, need=4, remains=%zu",
                    stream.in_remain());
                throw Error(ERR_LIC);
            }
            this->licenseInfo.cbCompanyName = stream.in_uint32_le();
            if (!stream.in_check_rem(this->licenseInfo.cbCompanyName)){
                LOG(LOG_ERR, "Licence UpgradeLicense_Recv : Truncated license info pbCompanyName, need=%u, remains=%zu",
                    this->licenseInfo.cbCompanyName, stream.in_remain());
                throw Error(ERR_LIC);
            }
            stream.in_copy_bytes(this->licenseInfo.pbCompanyName, this->licenseInfo.cbCompanyName);

            if (!stream.in_check_rem(4)){
                LOG(LOG_ERR, "Licence UpgradeLicense_Recv : Truncated license info cbProductId, need=4, remains=%zu",
                    stream.in_remain());
                throw Error(ERR_LIC);
            }
            this->licenseInfo.cbProductId = stream.in_uint32_le();
            if (!stream.in_check_rem(this->licenseInfo.cbProductId)){
                LOG(LOG_ERR, "Licence UpgradeLicense_Recv : Truncated license info pbProductId, need=%u, remains=%zu",
                    this->licenseInfo.cbProductId, stream.in_remain());
                throw Error(ERR_LIC);
            }
            stream.in_copy_bytes(this->licenseInfo.pbProductId, this->licenseInfo.cbProductId);

            if (!stream.in_check_rem(4)){
                LOG(LOG_ERR, "Licence UpgradeLicense_Recv : Truncated license info cbLicenseInfo, need=4, remains=%zu",
                    stream.in_remain());
                throw Error(ERR_LIC);
            }
            this->licenseInfo.cbLicenseInfo = stream.in_uint32_le();
            if (!stream.in_check_rem(this->licenseInfo.cbLicenseInfo)){
                LOG(LOG_ERR, "Licence UpgradeLicense_Recv : Truncated license info pbLicenseInfo, need=%u, remains=%zu",
                    this->licenseInfo.cbLicenseInfo, stream.in_remain());
                throw Error(ERR_LIC);
            }
            stream.in_copy_bytes(this->licenseInfo.pbLicenseInfo, this->licenseInfo.cbLicenseInfo);

            if (!stream.in_check_rem(LICENSE_SIGNATURE_SIZE)){
                LOG(LOG_ERR, "Licence UpgradeLicense_Recv : Truncated license info MACData, need=%u, remains=%zu",
                    LICENSE_SIGNATURE_SIZE, stream.in_remain());
                throw Error(ERR_LIC);
            }
            stream.in_copy_bytes(this->MACData, LICENSE_SIGNATURE_SIZE);

            if (stream.in_remain()){
                LOG(LOG_ERR, "UpgradeLicense_Recv : unparsed data %zu", stream.in_remain());
                throw Error(ERR_LIC);
            }
        }
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

// +-------------------------------------+---------------------------------------------------------------------------+
// | 0x0F LicenseProtocolVersionMask     | The license protocol version. See the discussion which follows this table |
// |                                     | for more information.                                                     |
// +-------------------------------------+---------------------------------------------------------------------------+
// | 0x80 EXTENDED_ERROR_MSG_SUPPORTED   | Indicates that extended error information using the License Error Message |
// |                                     | (section 2.2.1.12.1.3) is supported.                                      |
// +-------------------------------------+---------------------------------------------------------------------------+

// The LicenseProtocolVersionMask is a 4-bit value containing the supported license protocol version.
// The following are possible version values.

// +-----+----------------------+------------------------------------------------+
// | 0x2 | PREAMBLE_VERSION_2_0 | RDP 4.0                                        |
// +-----+----------------------+------------------------------------------------+
// | 0x3 | PREAMBLE_VERSION_3_0 | RDP 5.0, 5.1, 5.2, 6.0, 6.1, 7.0, 7.1, and 8.0 |
// +-----+----------------------+------------------------------------------------+

// wMsgSize (2 bytes): An 16-bit, unsigned integer. The size in bytes of the licensing packet
// (including the size of the preamble).

// 2.2.1.12.1.3 Licensing Error Message (LICENSE_ERROR_MESSAGE)
// ===========================================================

// The LICENSE_ERROR_MESSAGE structure is used to indicate that an error occurred during the licensing protocol.
// Alternatively, it is also used to notify the peer of important status information.

// dwErrorCode (4 bytes): A 32-bit, unsigned integer. The error or status code.

//    Sent by client:
//    ERR_INVALID_SERVER_CERTIFICATE   0x00000001
//    ERR_NO_LICENSE                   0x00000002

//    Sent by server:
//    ERR_INVALID_SCOPE                0x00000004
//    ERR_NO_LICENSE_SERVER            0x00000006
//    STATUS_VALID_CLIENT              0x00000007
//    ERR_INVALID_CLIENT               0x00000008
//    ERR_INVALID_PRODUCTID            0x0000000B
//    ERR_INVALID_MESSAGE_LEN          0x0000000C

//    Sent by client and server:
//    ERR_INVALID_MAC                  0x00000003

// dwStateTransition (4 bytes): A 32-bit, unsigned integer. The licensing state to transition into upon receipt
//    of this message. For more details about how this field is used, see [MS-RDPELE] section 3.1.5.2.

//    ST_TOTAL_ABORT            0x00000001
//    ST_NO_TRANSITION          0x00000002
//    ST_RESET_PHASE_TO_START   0x00000003
//    ST_RESEND_LAST_MESSAGE    0x00000004

//    3.1.5.2 Sending License Error Messages
//    ======================================

//     Both the client and the server can send a license error message. Whenever an error message is sent,
//     the message type in the licensing preamble MUST be set to ERROR_ALERT (0xFF). For the PDU, see [MS-RDPBCGR]
//     section 2.2.1.12.1.3

//     The client and the server MUST also set the appropriate state transition value in the dwStateTransition
//     field in the PDU. This is used to determine the next action to take. For state transitions,
//     see Processing License Error Messages.

//     A more detailed reason for the error MAY be passed by using the bbErrorInfo BLOB. The BLOB type MUST be
//     BB_ERROR_BLOB (see [MS-RDPBCGR] sections 2.2.1.12.1.2 and 2.2.1.12.1.3). This BLOB is empty if no detailed
//     reason for the error is passed.

// bbErrorInfo (variable): A LICENSE_BINARY_BLOB (section 2.2.1.12.1.2) structure which MUST contain a BLOB
//    of type BB_ERROR_BLOB (0x0004) that include " information relevant to the error code specified in dwErrorCode.

    struct ValidClientMessage
    {
        uint32_t dwErrorCode;
        uint32_t dwStateTransition;

        uint16_t wBlobType;
        uint16_t wBlobLen;

    };

    struct ErrorAlert_Recv
    {
        uint8_t wMsgType;
        uint8_t bVersion;
        uint16_t wMsgSize;

        // validClientMessage (variable): A Licensing Error Message (section 2.2.1.12.1.3) structure.
        // The dwStateTransition field MUST be set to ST_NO_TRANSITION (0x00000002).
        // The bbErrorInfo field MUST contain an empty binary large object (BLOB) of type BB_ERROR_BLOB (0x0004).
        ValidClientMessage validClientMessage;

        explicit ErrorAlert_Recv(InStream & stream) {
            hexdump_d(stream.get_data(), stream.get_capacity());
            const unsigned expected =
                16; /* wMsgType(1) + bVersion(1) + wMsgSize(2) + dwErrorCode(4) + dwStateTransition(4) + wBlobType(2) + wBlobLen(2) */
            if (!stream.in_check_rem(expected)){
                LOG(LOG_ERR, "Licence ErrorAlert_Recv : Truncated data, need=%u, remains=%zu",
                    expected, stream.in_remain());
                throw Error(ERR_LIC);
            }

            this->wMsgType = stream.in_uint8();
            this->bVersion = stream.in_uint8();
            this->wMsgSize = stream.in_uint16_le();

            this->validClientMessage.dwErrorCode = stream.in_uint32_le();
            this->validClientMessage.dwStateTransition = stream.in_uint32_le();
            this->validClientMessage.wBlobType = stream.in_uint16_le();
            this->validClientMessage.wBlobLen = stream.in_uint16_le();

            if (this->validClientMessage.dwStateTransition != ST_NO_TRANSITION) {
                LOG(LOG_ERR, "Unexpected dwStateTransition in Licence ErrorAlert_Recv expected ST_NO_TRANSITION, got %u",
                    this->validClientMessage.dwStateTransition);
            }
            // Ignore Blog Type if BlobLen is 0
            if ((this->validClientMessage.wBlobLen != 0)
            && (this->validClientMessage.wBlobType != LIC::BB_ERROR_BLOB)){
                LOG(LOG_ERR, "Unexpected BlobType in Licence ErrorAlert_Recv expected BB_ERROR_BLOB, got %u",
                    this->validClientMessage.wBlobType);
            }
            if (this->validClientMessage.wBlobLen != 0){
                LOG(LOG_ERR, "Unexpected BlobLen in Licence ErrorAlert_Recv expected empty blob, got %u bytes",
                    this->validClientMessage.wBlobLen);
            }
            if (stream.in_remain()){
                LOG(LOG_ERR, "Licence ErrorAlert_Recv : unparsed data %zu", stream.in_remain());
            }

            // wBlobType in Licence ErrorAlert_Recv is not 4 on windows 2000 or Windows XP... (content looks like garbage)
            if ((this->validClientMessage.dwStateTransition != ST_NO_TRANSITION)
//            || (this->validClientMessage.wBlobType != 4)
            || (this->validClientMessage.wBlobLen != 0)
            || stream.in_remain()){
                LOG(LOG_ERR,
                    "Licence ErrorAlert_Recv : "
                        "dwErrorCode=%u dwStateTransition=%u wBlobType=%u wBlobLen=%u",
                    this->validClientMessage.dwErrorCode,
                    this->validClientMessage.dwStateTransition,
                    static_cast<uint32_t>(this->validClientMessage.wBlobType),
                    static_cast<uint32_t>(this->validClientMessage.wBlobLen));
                throw Error(ERR_LIC);
            }
        }
    };

    struct ErrorAlert_Send
    {
        uint8_t wMsgType;
        uint8_t bVersion;
        uint16_t wMsgSize;

        // validClientMessage (variable): A Licensing Error Message (section 2.2.1.12.1.3) structure.
        // The dwStateTransition field MUST be set to ST_NO_TRANSITION (0x00000002).
        // The bbErrorInfo field MUST contain an empty binary large object (BLOB) of type BB_ERROR_BLOB (0x0004).
        ValidClientMessage validClientMessage;

        ErrorAlert_Send(OutStream & /*unused*/, uint8_t msgType, uint8_t version, ValidClientMessage & /*unused*/) :
        	wMsgType(msgType), bVersion(version), wMsgSize(0)
        {
        }
    };



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


    //    4.5 CLIENT PLATFORM CHALLENGE RESPONSE
    //    --------------------------------------
    //    The client sends the Client Platform Challenge Response message in response to a Server Platform Challenge message. The message type in the licensing preamble is PLATFORM_CHALLENGE_RESPONSE (0x15).

    //    See sections 2.2.2.5 and 3.3.5.5 for more information.

    //    00000000:  15 83 42 00 01 00 12 00-fa b4 e8 24 cf 56 b2 4e  ..B........$.V.N
    //    00000010:  80 02 bd b6 61 fc df e9-6c 44 01 00 14 00 f8 b5  ....a...lD......
    //    00000020:  e8 25 3d 0f 3f 70 1d da-60 19 16 fe 73 1a 45 7e  .%=.?p..`...s.E~
    //    00000030:  02 71 38 23 62 5d 10 8b-93 c3 f1 e4 67 1f 4a b6  .q8#b]......g.J.
    //    00000040:  00 0a                                            ..

    //    0x00: LICENSE_PREAMBLE (4 bytes)
    //    15 -> LICENSE_PREAMBLE::bMsgType = CLIENT_PLATFORM_CHALLENGE_REPSONSE
    //
    //    83 -> LICENSE_PREAMBLE::bVersion = 0x80 | 0x3
    //
    //    42 -\|
    //    00 -/ LICENSE_PREAMBLE::wMsgSize = 0x42 bytes
    //    0x04: EncryptedPlatformChallengeResponse
    //       (2 + 2 + 0x12 = 0x16 bytes)
    //    01 -\|
    //    00 -/ EncryptedPlatformChallengeResponse::wBlobType (ignored)
    //
    //    12 -\|
    //    00 -/ EncryptedPlatformChallengeResponse::wBlobLen = 0x12
    //
    //    fa b4 e8 24 cf 56 b2 4e -\|
    //    80 02 bd b6 61 fc df e9 -|
    //    6c 44                   -/
    //    EncryptedPlatformChallengeResponse::pBlob

    //    The corresponding decrypted platform challenge response data
    //    for the above is
    //    00 -\|
    //    01 -/ PLATFORM_CHALLENGE_RESPONSE_DATA::wVersion

    //    00 -\|
    //    01 -/ PLATFORM_CHALLENGE_RESPONSE_DATA::wClientType

    //    03 -\|
    //    00 -/ PLATFORM_CHALLENGE_RESPONSE_DATA::wLicenseDetailLevel

    //    0a -\|
    //    00 -/ PLATFORM_CHALLENGE_RESPONSE_DATA::cbChallenge

    //    54 -\|
    //    00 -|
    //    45 -|
    //    00 -|
    //    53 -|
    //    00 -|
    //    54 -|
    //    00 -|
    //    00 -|
    //    00 -/ PLATFORM_CHALLENGE_RESPONSE_DATA::pbChallenge

    //    0x1a: EncryptedHWID (2 + 2 + 0x14 = 0x18 bytes)
    //    01 -\|
    //    00 -/ EncryptedHWID::wBlobType (ignored)
    //
    //    14 -\|
    //    00 -/ EncryptedHWID::wBlobLen
    //
    //    f8 b5 e8 25 3d 0f 3f 70 -\|
    //    1d da-60 19 16 fe 73 1a -|
    //    45 7e 02 71             -/ EncryptedHWID::pBlob

    //    The corresponding decrypted HWID for the above is
    //    02 -\|
    //    00 -|
    //    00 -|
    //    00 -/ CLIENT_HARDWARE_ID::PlatformId

    //    f1 -\|
    //    59 -|
    //    87 -|
    //    3e -/ CLIENT_HARDWARE_ID::Data1

    //    c9 -\|
    //    d8 -|
    //    98 -|
    //    af -/ CLIENT_HARDWARE_ID::Data2

    //    24 -\|
    //    02 -|
    //    f8 -|
    //    f3 -/ CLIENT_HARDWARE_ID::Data3

    //    29 -\|
    //    3a -|
    //    f0 -|
    //    26 -/ CLIENT_HARDWARE_ID::Data4

    //    0x32: MACData (0x10 bytes)
    //    38 23 62 5d 10 8b-93 c3 -\|
    //    f1 e4 67 1f 4a b6 00 0a -/ MACData


    class ClientPlatformChallengeResponse_Send
    {
    public:
        ClientPlatformChallengeResponse_Send(OutStream & stream, int version, uint8_t * out_token, uint8_t * crypt_hwid, uint8_t * out_sig)
        {
            stream.out_uint8(LIC::PLATFORM_CHALLENGE_RESPONSE);
            stream.out_uint8(version); /* version */
            stream.out_uint16_le(12+LIC::LICENSE_TOKEN_SIZE+LIC::LICENSE_HWID_SIZE+LIC::LICENSE_SIGNATURE_SIZE);

            // wBlobType (2 bytes): A 16-bit, unsigned integer. The data type of
            // the binary information. If wBlobLen is set to 0, then the contents
            // of this field SHOULD be ignored.
            stream.out_uint16_le(LIC::BB_DATA_BLOB);

            // wBlobLen (2 bytes): A 16-bit, unsigned integer. The size in bytes of
            // the binary information in the blobData field. If wBlobLen is set to 0,
            // then the blobData field is not include " in the Licensing Binary BLOB
            // structure and the contents of the wBlobType field SHOULD be ignored.
            stream.out_uint16_le(LIC::LICENSE_TOKEN_SIZE);
            stream.out_copy_bytes(out_token, LIC::LICENSE_TOKEN_SIZE);

            // wBlobType (2 bytes): A 16-bit, unsigned integer. The data type of
            // the binary information. If wBlobLen is set to 0, then the contents
            // of this field SHOULD be ignored.
            stream.out_uint16_le(LIC::BB_DATA_BLOB);

            // wBlobLen (2 bytes): A 16-bit, unsigned integer. The size in bytes of
            // the binary information in the blobData field. If wBlobLen is set to 0,
            // then the blobData field is not include " in the Licensing Binary BLOB
            // structure and the contents of the wBlobType field SHOULD be ignored.
            stream.out_uint16_le(LIC::LICENSE_HWID_SIZE);
            stream.out_copy_bytes(crypt_hwid, LIC::LICENSE_HWID_SIZE);

            stream.out_copy_bytes(out_sig, LIC::LICENSE_SIGNATURE_SIZE);
        }
    };


} /* namespace LIC */


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

