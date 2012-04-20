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

   Generic Conference Control (T.124)

   T.124 GCC is defined in:
   
   http://www.itu.int/rec/T-REC-T.124-199802-S/en
   ITU-T T.124 (02/98): Generic Conference Control

*/

#if !defined(__CORE_RDP_GCC_HPP__)
#define __CORE_RDP_GCC_HPP__

#include "stream.hpp"

static inline void gcc_write_conference_create_request_header(Stream & stream, size_t & offset_length)
{
// ConnectData
    stream.out_per_choice(0); // From Key select object (0) of type OBJECT_IDENTIFIER
    const uint8_t t124_02_98_oid[6] = { 0, 0, 20, 124, 0, 1 };
    stream.out_per_object_identifier(t124_02_98_oid); // ITU-T T.124 (02/98) OBJECT_IDENTIFIER

//  ConnectData::connectPDU (OCTET_STRING)
    offset_length = stream.get_offset(0);
    stream.out_per_length(256); // connectPDU length (reserve 16 bits)

//  ConnectGCCPDU
    stream.out_per_choice(0); // From ConnectGCCPDU select conferenceCreateRequest (0) of type ConferenceCreateRequest
    stream.out_per_selection(0x08); // select optional userData from ConferenceCreateRequest

//  ConferenceCreateRequest::conferenceName
//	stream.out_per_numeric_string(s, (uint8*)"1", 1, 1); /* ConferenceName::numeric */
    stream.out_uint16_be(16);
    stream.out_per_padding(1); /* padding */

//  UserData (SET OF SEQUENCE)
    stream.out_per_number_of_sets(1); // one set of UserData
    stream.out_per_choice(0xC0); // UserData::value present + select h221NonStandard (1)

//  h221NonStandard
    const uint8_t h221_cs_key[4] = {'D', 'u', 'c', 'a'};
    stream.out_per_octet_string(h221_cs_key, 4, 4); // h221NonStandard, client-to-server H.221 key, "Duca"
}

#endif
