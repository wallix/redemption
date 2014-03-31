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
    Copyright (C) Wallix 2014
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#ifndef _REDEMPTION_CORE_RDP_PERSISTENTKEYLISTPDU_HPP_
#define _REDEMPTION_CORE_RDP_PERSISTENTKEYLISTPDU_HPP_

 #include "caches/bmpcache.hpp"

namespace RDP {

// [MS-RDPBCGR] - 2.2.1.17 Client Persistent Key List PDU
// ======================================================

// The Persistent Key List PDU is an RDP Connection Sequence PDU sent from
//  client to server during the Connection Finalization phase of the RDP
//  Connection Sequence (see section 1.3.1.1 for an overview of the RDP
//  Connection Sequence phases). A single Persistent Key List PDU or a
//  sequence of Persistent Key List PDUs MUST be sent after transmitting the
//  Client Control (Request Control) PDU (section 2.2.1.16) if the client has
//  bitmaps that were stored in a Persistent Bitmap Cache (section 3.2.1.14),
//  the server advertised support for the Bitmap Host Cache Support
//  Capability Set (section 2.2.7.2.1), and a Deactivation-Reactivation
//  Sequence is not in progress (see section 1.3.1.3 for an overview of the
//  Deactivation-Reactivation Sequence).

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                           tpktHeader                          |
// +-----------------------------------------------+---------------+
// |                    x224Data                   |    mcsSDrq    |
// |                                               |   (variable)  |
// +-----------------------------------------------+---------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                   securityHeader (variable)                   |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |              persistentKeyListPduData (variable)              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// tpktHeader (4 bytes): A TPKT Header, as specified in [T123] section 8.

// x224Data (3 bytes): An X.224 Class 0 Data TPDU, as specified in [X224]
//  section 13.7.

// mcsSDrq (variable): Variable-length PER-encoded MCS Domain PDU
//  (DomainMCSPDU), which encapsulates an MCS Send Data Request structure
//  (SDrq, choice 25 from DomainMCSPDU), as specified in [T125] section 11.32
//  (the ASN.1 structure definitions are given in [T125] section 7, parts 7
//  and 10). The userData field of the MCS Send Data Request contains a
//  Security Header and a Persistent Key List PDU Data (section 2.2.1.17.1)
//  structure.

// securityHeader (variable): Optional security header. The presence and
//  format of the security header depends on the Encryption Level and
//  Encryption Method selected by the server (sections 5.3.2 and 2.2.1.4.3).
//  If the Encryption Level selected by the server is greater than
//  ENCRYPTION_LEVEL_NONE (0) and the Encryption Method selected by the
//  server is greater than ENCRYPTION_METHOD_NONE (0), then this field MUST
//  contain one of the following headers:

//  * Non-FIPS Security Header (section 2.2.8.1.1.2.2) if the Encryption
//    Method selected by the server is ENCRYPTION_METHOD_40BIT (0x00000001),
//    ENCRYPTION_METHOD_56BIT (0x00000008), or ENCRYPTION_METHOD_128BIT
//    (0x00000002).

//  * FIPS Security Header (section 2.2.8.1.1.2.3) if the Encryption Method
//    selected by the server is ENCRYPTION_METHOD_FIPS (0x00000010).

//  If the Encryption Level selected by the server is ENCRYPTION_LEVEL_NONE
//   (0) and the Encryption Method selected by the server is
//   ENCRYPTION_METHOD_NONE (0), then this header MUST NOT be included in the
//   PDU.

// persistentKeyListPduData (variable): The contents of the Persistent Key
//  List PDU, as specified in section 2.2.1.17.1.

// [MS-RDPBCGR] - 2.2.1.17.1 Persistent Key List PDU Data
//  (TS_BITMAPCACHE_PERSISTENT_LIST_PDU)
// ======================================================

// The TS_BITMAPCACHE_PERSISTENT_LIST_PDU structure contains a list of cached
//  bitmap keys saved from Cache Bitmap (Revision 2) Orders (see [MS-RDPEGDI]
//  section 2.2.2.2.1.2.3) that were sent in previous sessions. By including
//  a key in the Persistent Key List PDU Data the client indicates to the
//  server that it has a local copy of the bitmap associated with the key,
//  which means that the server does not need to retransmit the bitmap to the
//  client (for more details about the Persistent Bitmap Cache, see
//  [MS-RDPEGDI] section 3.1.1.1.1). The bitmap keys can be sent in more than
//  one Persistent Key List PDU, with each PDU being marked using flags in
//  the bBitMask field. The number of bitmap keys encapsulated within the
//  Persistent Key List PDU Data SHOULD be limited to 169.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                        shareDataHeader                        |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +-------------------------------+-------------------------------+
// |              ...              |        numEntriesCache0       |
// +-------------------------------+-------------------------------+
// |        numEntriesCache1       |        numEntriesCache2       |
// +-------------------------------+-------------------------------+
// |        numEntriesCache3       |        numEntriesCache4       |
// +-------------------------------+-------------------------------+
// |       totalEntriesCache0      |       totalEntriesCache1      |
// +-------------------------------+-------------------------------+
// |       totalEntriesCache2      |       totalEntriesCache3      |
// +-------------------------------+---------------+---------------+
// |       totalEntriesCache4      |    bBitMask   |      Pad2     |
// +-------------------------------+---------------+---------------+
// |              Pad3             |       entries (variable)      |
// +-------------------------------+-------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// shareDataHeader (18 bytes): Share Data Header (section 2.2.8.1.1.1.2)
//  containing information about the packet. The type subfield of the pduType
//  field of the Share Control Header (section 2.2.8.1.1.1.1) MUST be set to
//  PDUTYPE_DATAPDU (7). The pduType2 field of the Share Data Header MUST be
//  set to PDUTYPE2_BITMAPCACHE_PERSISTENT_LIST (43).

// numEntriesCache0 (2 bytes): A 16-bit, unsigned integer. The number of
//  entries for Bitmap Cache 0 in the current Persistent Key List PDU.

// numEntriesCache1 (2 bytes): A 16-bit, unsigned integer. The number of
//  entries for Bitmap Cache 1 in the current Persistent Key List PDU.

// numEntriesCache2 (2 bytes): A 16-bit, unsigned integer. The number of
//  entries for Bitmap Cache 2 in the current Persistent Key List PDU.

// numEntriesCache3 (2 bytes): A 16-bit, unsigned integer. The number of
//  entries for Bitmap Cache 3 in the current Persistent Key List PDU.

// numEntriesCache4 (2 bytes): A 16-bit, unsigned integer. The number of
//  entries for Bitmap Cache 4 in the current Persistent Key List PDU.

// totalEntriesCache0 (2 bytes): A 16-bit, unsigned integer. The total
//  number of entries for Bitmap Cache 0 expected across the entire sequence
//  of Persistent Key List PDUs. This value MUST remain unchanged across the
//  sequence. The sum of the totalEntries0, totalEntries1, totalEntries2,
//  totalEntries3, and totalEntries4 fields MUST NOT exceed 262,144.

// totalEntriesCache1 (2 bytes): A 16-bit, unsigned integer. The total number
//  of entries for Bitmap Cache 1 expected across the entire sequence of
//  Persistent Key List PDUs. This value MUST remain unchanged across the
//  sequence. The sum of the totalEntries0, totalEntries1, totalEntries2,
//  totalEntries3, and totalEntries4 fields MUST NOT exceed 262,144.

// totalEntriesCache2 (2 bytes): A 16-bit, unsigned integer. The total number
//  of entries for Bitmap Cache 2 expected across the entire sequence of
//  Persistent Key List PDUs. This value MUST remain unchanged across the
//  sequence. The sum of the totalEntries0, totalEntries1, totalEntries2,
//  totalEntries3, and totalEntries4 fields MUST NOT exceed 262,144.

// totalEntriesCache3 (2 bytes): A 16-bit, unsigned integer. The total number
//  of entries for Bitmap Cache 3 expected across the entire sequence of
//  Persistent Key List PDUs. This value MUST remain unchanged across the
//  sequence. The sum of the totalEntries0, totalEntries1, totalEntries2,
//  totalEntries3, and totalEntries4 fields MUST NOT exceed 262,144.

// totalEntriesCache4 (2 bytes): A 16-bit, unsigned integer. The total number
//  of entries for Bitmap Cache 4 expected across the entire sequence of
//  Persistent Key List PDUs. This value MUST remain unchanged across the
//  sequence.

// bBitMask (1 byte): An 8-bit, unsigned integer. The sequencing flag.

//  +-------------------+------------------------------------------------------+
//  | Flag              | Meaning                                              |
//  +-------------------+------------------------------------------------------+
//  | PERSIST_FIRST_PDU | Indicates that the PDU is the first in a sequence of |
//  | 0x01              | Persistent Key List PDUs.                            |
//  +-------------------+------------------------------------------------------+
//  | PERSIST_LAST_PDU  | Indicates that the PDU is the last in a sequence of  |
//  | 0x02              | Persistent Key List PDUs.                            |
//  +-------------------+------------------------------------------------------+

enum {
      PERSIST_FIRST_PDU = 0x01
    , PERSIST_LAST_PDU  = 0x02
};

//  If neither PERSIST_FIRST_PDU (0x01) nor PERSIST_LAST_PDU (0x02) are set,
//   then the current PDU is an intermediate packet in a sequence of
//   Persistent Key List PDUs.

// Pad2 (1 byte): An 8-bit, unsigned integer. Padding. Values in this field
//  MUST be ignored.

// Pad3 (2 bytes): A 16-bit, unsigned integer. Padding. Values in this field
//  MUST be ignored.

// entries (variable): An array of TS_BITMAPCACHE_PERSISTENT_LIST_ENTRY
//  structures which describe 64-bit bitmap keys. The keys MUST be arranged
//  in order from low cache number to high cache number. For instance, if a
//  PDU contains one key for Bitmap Cache 0 and two keys for Bitmap Cache 1,
//  then numEntriesCache0 will be set to 1, numEntriesCache1 will be set to
//  2, and numEntriesCache2, numEntriesCache3, and numEntriesCache4 will all
//  be set to zero. The keys will be arranged in the following order: (Bitmap
//  Cache 0, Key 1), (Bitmap Cache 1, Key 1), (Bitmap Cache 1, Key 2).

// [MS-RDPBCGR] - 2.2.1.17.1.1 Persistent List Entry
//  (TS_BITMAPCACHE_PERSISTENT_LIST_ENTRY)
// =================================================

// The TS_BITMAPCACHE_PERSISTENT_LIST_ENTRY structure contains a 64-bit
//  bitmap key to be sent back to the server.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                              Key1                             |
// +---------------------------------------------------------------+
// |                              Key2                             |
// +---------------------------------------------------------------+

// Key1 (4 bytes): Low 32 bits of the 64-bit persistent bitmap cache key.

// Key2 (4 bytes): A 32-bit, unsigned integer. High 32 bits of the 64-bit
//  persistent bitmap cache key.

struct BitmapCachePersistentListEntry {
    uint32_t Key1;
    uint32_t Key2;
};

struct PersistentKeyListPDUData {
    static const size_t MAXIMUM_ENCAPSULATED_BITMAP_KEYS = 169;

    uint16_t number_entries_cache[5];

    uint16_t & numEntriesCache0;
    uint16_t & numEntriesCache1;
    uint16_t & numEntriesCache2;
    uint16_t & numEntriesCache3;
    uint16_t & numEntriesCache4;
    uint16_t   totalEntriesCache0;
    uint16_t   totalEntriesCache1;
    uint16_t   totalEntriesCache2;
    uint16_t   totalEntriesCache3;
    uint16_t   totalEntriesCache4;
    uint8_t    bBitMask;

    BitmapCachePersistentListEntry entries[MAXIMUM_ENCAPSULATED_BITMAP_KEYS];

    PersistentKeyListPDUData()
    : numEntriesCache0(number_entries_cache[0])
    , numEntriesCache1(number_entries_cache[1])
    , numEntriesCache2(number_entries_cache[2])
    , numEntriesCache3(number_entries_cache[3])
    , numEntriesCache4(number_entries_cache[4])
    , totalEntriesCache0(0)
    , totalEntriesCache1(0)
    , totalEntriesCache2(0)
    , totalEntriesCache3(0)
    , totalEntriesCache4(0)
    , bBitMask(0)
    {
        ::memset(number_entries_cache, 0, sizeof(number_entries_cache));
        ::memset(entries, 0, sizeof(entries));
    }

    void receive(Stream & stream) {
        unsigned expected = 24; /* numEntriesCache0(2) + numEntriesCache1(2) + numEntriesCache2(2) +
                                   numEntriesCache3(2) + numEntriesCache4(2) + totalEntriesCache0(2) +
                                   totalEntriesCache1(2) + totalEntriesCache2(2) + totalEntriesCache3(2) +
                                   totalEntriesCache4(2) + bBitMask(1) + Pad2(1) + Pad3(2) */
        if (!stream.in_check_rem(expected)) {
            LOG( LOG_ERR
               , "PersistentKeyListPDUData::receive  - Truncated data, need=%u, remains=%u"
               , expected, stream.in_remain());
            throw Error(ERR_RDP_DATA_TRUNCATED);
        }

        this->numEntriesCache0   = stream.in_uint16_le();
        this->numEntriesCache1   = stream.in_uint16_le();
        this->numEntriesCache2   = stream.in_uint16_le();
        this->numEntriesCache3   = stream.in_uint16_le();
        this->numEntriesCache4   = stream.in_uint16_le();
        this->totalEntriesCache0 = stream.in_uint16_le();
        this->totalEntriesCache1 = stream.in_uint16_le();
        this->totalEntriesCache2 = stream.in_uint16_le();
        this->totalEntriesCache3 = stream.in_uint16_le();
        this->totalEntriesCache4 = stream.in_uint16_le();
        this->bBitMask           = stream.in_uint8();

        stream.in_skip_bytes(3);    // Pad2(1) + Pad3(2)

        unsigned int count = std::min<uint32_t>( this->numEntriesCache0 +
                                                 this->numEntriesCache1 +
                                                 this->numEntriesCache2 +
                                                 this->numEntriesCache3 +
                                                 this->numEntriesCache4
                                               , MAXIMUM_ENCAPSULATED_BITMAP_KEYS);

        expected = count * 8; /* count * (Key1(4) + Key2(4)) */
        if (!stream.in_check_rem(expected)) {
            LOG( LOG_ERR
               , "PersistentKeyListPDUData::receive  - Truncated entries, need=%u, remains=%u"
               , expected, stream.in_remain());
            throw Error(ERR_RDP_DATA_TRUNCATED);
        }

        for (unsigned int index = 0; index < count; index++) {
            this->entries[index].Key1 = stream.in_uint32_le();
            this->entries[index].Key2 = stream.in_uint32_le();
        }
    }

    void emit(Stream & stream) {
        stream.out_uint16_le(this->numEntriesCache0);
        stream.out_uint16_le(this->numEntriesCache1);
        stream.out_uint16_le(this->numEntriesCache2);
        stream.out_uint16_le(this->numEntriesCache3);
        stream.out_uint16_le(this->numEntriesCache4);
        stream.out_uint16_le(this->totalEntriesCache0);
        stream.out_uint16_le(this->totalEntriesCache1);
        stream.out_uint16_le(this->totalEntriesCache2);
        stream.out_uint16_le(this->totalEntriesCache3);
        stream.out_uint16_le(this->totalEntriesCache4);
        stream.out_uint8(this->bBitMask);

        stream.out_clear_bytes(3);  // Pad2(1) + Pad3(2)

        for (uint32_t i = 0,
                      c = std::min<uint32_t>(this->numEntriesCache0 +
                                             this->numEntriesCache1 +
                                             this->numEntriesCache2 +
                                             this->numEntriesCache3 +
                                             this->numEntriesCache4,
                                             MAXIMUM_ENCAPSULATED_BITMAP_KEYS);
             i < c; i++) {
            stream.out_uint32_le(this->entries[i].Key1);
            stream.out_uint32_le(this->entries[i].Key2);
        }
    }

    void reset() {
        this->numEntriesCache0   = 0;
        this->numEntriesCache1   = 0;
        this->numEntriesCache2   = 0;
        this->numEntriesCache3   = 0;
        this->numEntriesCache4   = 0;
        this->totalEntriesCache0 = 0;
        this->totalEntriesCache1 = 0;
        this->totalEntriesCache2 = 0;
        this->totalEntriesCache3 = 0;
        this->totalEntriesCache4 = 0;
        this->bBitMask           = 0;
        ::memset(entries, 0, sizeof(entries));
    }

    void log(int level, const char * message) const {
        char   buffer[2048];
        size_t sz = sizeof(buffer);
        size_t lg = 0;
        lg += snprintf(buffer + lg, sz - lg,
            "%s PersistentKeyListPDUData(numEntriesCache0=%u numEntriesCache1=%u numEntriesCache2=%u numEntriesCache3=%u "
                "numEntriesCache4=%u totalEntriesCache0=%u totalEntriesCache1=%u totalEntriesCache2=%u"
                " totalEntriesCache3=%u totalEntriesCache4=%u bBitMask=%u entries(",
            message,
            this->numEntriesCache0, this->numEntriesCache1, this->numEntriesCache2, this->numEntriesCache3,
            this->numEntriesCache4, this->totalEntriesCache0, this->totalEntriesCache1, this->totalEntriesCache2,
            this->totalEntriesCache3, this->totalEntriesCache4, this->bBitMask);
        for (uint32_t i = 0,
                      c = std::min<uint32_t>(this->numEntriesCache0 +
                                             this->numEntriesCache1 +
                                             this->numEntriesCache2 +
                                             this->numEntriesCache3 +
                                             this->numEntriesCache4,
                                             MAXIMUM_ENCAPSULATED_BITMAP_KEYS);
             i < c; i++) {
            if (i) {
                lg += snprintf(buffer + lg, sz - lg, " ");
            }

            if (i > 20) {
                lg += snprintf(buffer + lg, sz - lg, "...");
                break;
            }

            uint8_t keys[8];
            memcpy(keys,     &this->entries[i].Key1, 4);
            memcpy(keys + 4, &this->entries[i].Key2, 4);
            lg += snprintf(buffer + lg, sz - lg, "(%02X%02X%02X%02X%02X%02X%02X%02X)", keys[0], keys[1],
                keys[2], keys[3], keys[4], keys[5], keys[6], keys[7]);
        }
        lg += snprintf(buffer + lg, sz - lg, "))");
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, buffer);
    }
};  // PersistentKeyListPDUData

const size_t PersistentKeyListPDUData::MAXIMUM_ENCAPSULATED_BITMAP_KEYS;

}   // namespace RDP

#endif  // #ifndef _REDEMPTION_CORE_RDP_PERSISTENTKEYLISTPDU_HPP_
