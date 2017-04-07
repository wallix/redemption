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
   Copyright (C) Wallix 2016
   Author(s): Christophe Grosjean

   Generic Conference Control (T.124)

   T.124 GCC is defined in:

   http://www.itu.int/rec/T-REC-T.124-199802-S/en
   ITU-T T.124 (02/98): Generic Conference Control

*/

#pragma once

#include "core/RDP/gcc/data_block_type.hpp"
#include "utils/stream.hpp"
#include "core/error.hpp"

namespace GCC { namespace UserData {

// 2.2.1.3.3 Client Security Data (TS_UD_CS_SEC)
// ---------------------------------------------
// The TS_UD_CS_SEC data block contains security-related information used to
// advertise client cryptographic support. This information is only relevant
// when Standard RDP Security mechanisms (section 5.3) will be used. See
// sections 3 and 5.3.2 for a detailed discussion of how this information is
// used.

// header (4 bytes): GCC user data block header as described in User Data
//                   Header (section 2.2.1.3.1). The User Data Header type
//                   field MUST be set to CS_SECURITY (0xC002).

// encryptionMethods (4 bytes): A 32-bit, unsigned integer. Cryptographic
//                              encryption methods supported by the client
//                              and used in conjunction with Standard RDP
//                              Security The server MUST select one of these
//                              methods. Section 5.3.2 describes how the
//                              client and server negotiate the security
//                              parameters for a given connection.
//
//           Value                           Meaning
// -------------------------------------------------------------------------
//    40BIT_ENCRYPTION_FLAG   40-bit session keys MUST be used to encrypt
//       0x00000001           data (with RC4) and generate Message
//                            Authentication Codes (MAC).
// -------------------------------------------------------------------------
//    128BIT_ENCRYPTION_FLAG  128-bit session keys MUST be used to encrypt
//       0x00000002           data (with RC4) and generate MACs.
// -------------------------------------------------------------------------
//    56BIT_ENCRYPTION_FLAG   56-bit session keys MUST be used to encrypt
//       0x00000008           data (with RC4) and generate MACs.
// -------------------------------------------------------------------------
//   FIPS_ENCRYPTION_FLAG All encryption and Message Authentication Code
//                            generation routines MUST be Federal
//       0x00000010           Information Processing Standard (FIPS) 140-1
//                            compliant.

// extEncryptionMethods (4 bytes): A 32-bit, unsigned integer. This field is
//                               used exclusively for the French locale.
//                               In French locale clients, encryptionMethods
//                               MUST be set to 0 and extEncryptionMethods
//                               MUST be set to the value to which
//                               encryptionMethods would have been set.
//                               For non-French locale clients, this field
//                               MUST be set to 0

struct CSSecurity {
    enum {
          _40BIT_ENCRYPTION_FLAG = 0x01
        , _128BIT_ENCRYPTION_FLAG = 0x02
        , _56BIT_ENCRYPTION_FLAG = 0x08
        , FIPS_ENCRYPTION_FLAG = 0x10
    };

    uint16_t userDataType;
    uint16_t length;
    uint32_t encryptionMethods;
    uint32_t extEncryptionMethods;

    CSSecurity()
    : userDataType(CS_SECURITY)
    , length(12)
    , encryptionMethods(_40BIT_ENCRYPTION_FLAG | _128BIT_ENCRYPTION_FLAG)
    , extEncryptionMethods(0)
    {
    }

    void emit(OutStream & stream)
    {
        stream.out_uint16_le(this->userDataType);
        stream.out_uint16_le(this->length);
        stream.out_uint32_le(this->encryptionMethods);
        stream.out_uint32_le(this->extEncryptionMethods);
    }

    void recv(InStream & stream)
    {
        if (!stream.in_check_rem(12)){
            LOG(LOG_ERR, "CS_SECURITY short header");
            throw Error(ERR_GCC);
        }
        this->userDataType         = stream.in_uint16_le();
        this->length               = stream.in_uint16_le();

        if (this->length != 12){
            LOG(LOG_ERR, "CS_SECURITY bad header length=%d", this->length);
            throw Error(ERR_GCC);
        }

        this->encryptionMethods    = stream.in_uint32_le();
        this->extEncryptionMethods = stream.in_uint32_le();
    }

    void log(const char * msg)
    {
        // --------------------- Base Fields ---------------------------------------
        LOG(LOG_INFO, "%s GCC User Data CS_SECURITY (%u bytes)", msg, this->length);
        LOG(LOG_INFO, "CSSecGccUserData::encryptionMethods %u", this->encryptionMethods);
        LOG(LOG_INFO, "CSSecGccUserData::extEncryptionMethods %u", this->extEncryptionMethods);
    }
};

}} // namespaces
