/*
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

Product name: redemption, a FLOSS RDP proxy
Copyright (C) Wallix 2021
Author(s): Proxies Team
*/

#pragma once

#include "utils/stream.hpp"
#include "core/error.hpp"

#include <vector>

namespace BER
{
    enum {
        CLASS_UNIV            = 0x00,
        CLASS_APPL            = 0x40,
        CLASS_CTXT            = 0x80,
        CLASS_MASK            = 0xC0,
        PC_MASK               = 0x20,
        PC_CONSTRUCT          = 0x20,
        PC_PRIMITIVE          = 0x00,
        TAG_MASK              = 0x1F,
        TAG_BOOLEAN           = 0x01,
        TAG_INTEGER           = 0x02,
        TAG_BIT_STRING        = 0x03,
        TAG_OCTET_STRING      = 0x04,
        TAG_OBJECT_IDENTIFIER = 0x06,
        TAG_ENUMERATED        = 0x0A,
        TAG_SEQUENCE_OF       = 0x10,
        TAG_GENERAL_STRING    = 0x1B,
    };

    std::vector<uint8_t> mkOptionalNegoTokensHeader(uint32_t payload_size);

    std::vector<uint8_t> mkContextualFieldHeader(uint32_t payload_size, uint8_t tag);

    std::vector<uint8_t> mkOctetStringHeader(uint32_t payload_size);

    std::vector<uint8_t> mkOidField(buffer_view oid, uint8_t tag);

    std::vector<uint8_t> mkMandatoryOctetStringFieldHeader(uint32_t payload_size, uint8_t tag);

    std::vector<uint8_t> mkOptionalOctetStringFieldHeader(uint32_t payload_size, uint8_t tag);

    std::vector<uint8_t> mkSmallIntegerField(uint8_t value, uint8_t tag);

    std::vector<uint8_t> mkIntegerField(uint32_t value, uint8_t tag);

    std::vector<uint8_t> mkSequenceHeader(uint32_t payload_size);

    std::vector<uint8_t> mkEnumeratedField(uint32_t value, uint8_t tag);

    bool check_ber_ctxt_tag(bytes_view s, uint8_t tag);

    std::pair<uint32_t, bytes_view> pop_length(bytes_view s, const char * message, error_type eid);

    bool check_ber_app_tag(bytes_view s, uint8_t tag, bool verbose, const char * message, bytes_view & body);

    bool peek_construct_tag(bytes_view s, uint8_t & tag, bool verbose, const char * message, bytes_view & queue);

    std::pair<uint32_t, bytes_view> pop_tag_length(bytes_view s, uint8_t tag, const char * message, error_type eid);

    std::pair<uint32_t, bytes_view> pop_integer(bytes_view s, const char * message, error_type eid);

    std::pair<uint32_t, bytes_view> pop_integer_field(bytes_view s, uint8_t tag, const char * message, error_type eid);

    uint32_t read_mandatory_integer(InStream & stream, uint8_t tag, const char * message, error_type eid);

    uint32_t read_optional_integer(InStream & stream, uint8_t tag, const char * message, error_type eid);

    std::pair<uint32_t, bytes_view> pop_enumerated(bytes_view s, const char * message, error_type eid);

    uint32_t read_mandatory_enumerated(InStream & stream, uint8_t tag, const char *message, error_type eid);

    void read_optional_enumerated(InStream & stream, uint8_t tag, uint32_t & ret, const char * message, error_type eid);

    std::vector<uint8_t> read_mandatory_octet_string(InStream & stream, uint8_t tag, const char * message, error_type eid);

    std::vector<uint8_t> read_optional_octet_string(InStream & stream, uint8_t tag, const char * message, error_type eid);

    typedef std::vector<uint8_t> BerOID;

    bool peek_oid(bytes_view s, bool verbose, const char * message, BerOID & oid, bytes_view & queue);

    std::pair<BerOID, bytes_view> pop_oid(bytes_view s, const char * message, error_type eid);

    BerOID pop_oid(InStream & s, const char * message, error_type eid);
} // namespace BER
