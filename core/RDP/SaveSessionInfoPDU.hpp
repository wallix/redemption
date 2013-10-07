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
    Copyright (C) Wallix 2013
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#ifndef _REDEMPTION_CORE_RDP_SAVE_SESSION_INFO_PDU_HPP_
#define _REDEMPTION_CORE_RDP_SAVE_SESSION_INFO_PDU_HPP_

namespace RDP {

// 2.2.10.1.1 Save Session Info PDU Data (TS_SAVE_SESSION_INFO_PDU_DATA)
// =====================================================================
// The TS_SAVE_SESSION_INFO_PDU_DATA structure is a wrapper around different
//  classes of user logon information.

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
// |              ...              |            infoType           |
// +-------------------------------+-------------------------------+
// |              ...              |      infoData (variable)      |
// +-------------------------------+-------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// shareDataHeader (18 bytes): Share Data Header containing information about
//  the packet. The type subfield of the pduType field of the Share Control
//  Header (section 2.2.8.1.1.1.1) MUST be set to PDUTYPE_DATAPDU (7). The
//  pduType2 field of the Share Data Header MUST be set to
//  PDUTYPE2_SAVE_SESSION_INFO (38).

// infoType (4 bytes): A 32-bit, unsigned integer. The type of logon
//  information.

// +------------------------------+--------------------------------------------+
// | Value                        | Meaning                                    |
// +------------------------------+--------------------------------------------+
// | INFOTYPE_LOGON               | This is a notification that the user has   |
// | 0x00000000                   | logged on. The infoData field which        |
// |                              | follows contains a Logon Info Version 1    |
// |                              | (section 2.2.10.1.1.1) structure.          |
// +------------------------------+--------------------------------------------+
// | INFOTYPE_LOGON_LONG          | This is a notification that the user has   |
// | 0x00000001                   | logged on. The infoData field which        |
// |                              | follows contains a Logon Info Version2     |
// |                              | (section 2.2.10.1.1.2) structure. This     |
// |                              | type is supported by RDP 5.1, 5.2, 6.0,    |
// |                              | 6.1, 7.0, 7.1, and 8.0 and SHOULD be used  |
// |                              | if the LONG_CREDENTIALS_SUPPORTED          |
// |                              | (0x00000004) flag is set in the General    |
// |                              | Capability Set (section 2.2.7.1.1).        |
// +------------------------------+--------------------------------------------+
// | INFOTYPE_LOGON_PLAINNOTIFY   | This is a notification that the user has   |
// | 0x00000002                   | logged on. The infoData field which        |
// |                              | follows contains a Plain Notify structure  |
// |                              | which contains 576 bytes of padding (see   |
// |                              | section 2.2.10.1.1.3). This type is        |
// |                              | supported by RDP 5.1, 5.2, 6.0, 6.1, 7.0,  |
// |                              | 7.1, and 8.0.                              |
// +------------------------------+--------------------------------------------+
// | INFOTYPE_LOGON_EXTENDED_INFO | The infoData field which follows contains  |
// | 0x00000003                   | a Logon Info Extended (section             |
// |                              | 2.2.10.1.1.4) structure. This type is      |
// |                              | supported by RDP 5.2, 6.0, 6.1, 7.0, 7.1,  |
// |                              | and 8.0.                                   |
// +------------------------------+--------------------------------------------+

enum {
    INFOTYPE_LOGON               = 0,
    INFOTYPE_LOGON_LONG          = 1,
    INFOTYPE_LOGON_PLAINNOTIFY   = 2,
    INFOTYPE_LOGON_EXTENDED_INFO = 3
};

// infoData (variable): A Logon Info Version 1 (section 2.2.10.1.1.1), Logon
//  Info Version 2 (section 2.2.10.1.1.2), Plain Notify (section
//  2.2.10.1.1.3), or Logon Info Extended (section 2.2.10.1.1.4) structure.
//  The type of data that follows depends on the value of the infoType field.

struct SaveSessionInfoPDUData_Recv {
    uint32_t infoType;

    SubStream payload;

    SaveSessionInfoPDUData_Recv(Stream & stream) :
    infoType(INFOTYPE_LOGON),
    payload(stream) {
        const unsigned expected = 4;    // infoType(4)

        if (!stream.in_check_rem(expected)) {
            LOG(LOG_ERR,
                "Truncated Save Session Info PDU (data): expected=%u remains=%u",
                expected, stream.in_remain());
            throw Error(ERR_RDP_DATA_TRUNCATED);
        }

        this->infoType = stream.in_uint32_le();

        this->payload.resize(stream, stream.in_remain());
    }
};

/*
struct SaveSessionInfoPDUData_Send {
    SaveSessionInfoPDUData_Send(Stream & stream, uint32_t infoType) {
        stream.out_uint32_le(infoType);
    }
};
*/

// 2.2.10.1.1.1 Logon Info Version 1 (TS_LOGON_INFO)
// =================================================
// TS_LOGON_INFO is a fixed-length structure that contains logon information
//  intended for the client.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                            cbDomain                           |
// +---------------------------------------------------------------+
// |                             Domain                            |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                  ( Domain cont'd for 5 rows)                  |
// +---------------------------------------------------------------+
// |                           cbUserName                          |
// +---------------------------------------------------------------+
// |                            UserName                           |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                ( UserName cont'd for 120 rows)                |
// +---------------------------------------------------------------+
// |                           SessionId                           |
// +---------------------------------------------------------------+

// cbDomain (4 bytes): A 32-bit, unsigned integer. The size of the Unicode
//  character data (including the mandatory null terminator) in bytes present
//  in the fixed-length Domain field.

// Domain (52 bytes): An array of 26 Unicode characters: Null-terminated
//  Unicode string containing the name of the domain to which the user is
//  logged on. The length of the character data in bytes is given by the
//  cbDomain field.

// cbUserName (4 bytes): A 32-bit, unsigned integer. Size of the Unicode
//  character data (including the mandatory null terminator) in bytes present
//  in the fixed-length UserName field.

// UserName (512 bytes): An array of 256 Unicode characters: Null-terminated
//  Unicode string containing the username which was used to log on. The
//  length of the character data in bytes is given by the cbUserName field.

// SessionId (4 bytes): A 32-bit, unsigned integer. Optional ID of the
//  session on the remote server according to the server. Sent by RDP 5.0,
//  5.1, 5.2, 6.0, 6.1, 7.0, 7.1, and 8.0 servers.

struct LogonInfoVersion1_Recv {
    uint32_t cbDomain;
    uint8_t  Domain[256];
    uint32_t cbUserName;
    uint8_t  UserName[2048];
    uint32_t SessionId;

    LogonInfoVersion1_Recv(Stream & stream) :
    cbDomain(0),
    cbUserName(0),
    SessionId(0) {
        memset(Domain,   0, sizeof(Domain));
        memset(UserName, 0, sizeof(UserName));

        unsigned expected = 4;  // cbDomain(4)
        if (!stream.in_check_rem(expected)) {
            LOG(LOG_ERR,
                "Truncated Logon Info Version 1 (data): expected=%u remains=%u",
                expected, stream.in_remain());
            throw Error(ERR_RDP_DATA_TRUNCATED);
        }

        this->cbDomain = stream.in_uint32_le();

        expected = this->cbDomain +
                   4;               // cbUserName(4)
        if (!stream.in_check_rem(expected)) {
            LOG(LOG_ERR,
                "Truncated Logon Info Version 1 (data): expected=%u remains=%u",
                expected, stream.in_remain());
            throw Error(ERR_RDP_DATA_TRUNCATED);
        }

        stream.in_uni_to_ascii_str(this->Domain, this->cbDomain,
            sizeof(this->Domain));

        this->cbUserName = stream.in_uint32_le();

        expected = this->cbUserName +
                   4;                   // SessionId(4)
        if (!stream.in_check_rem(expected)) {
            LOG(LOG_ERR,
                "Truncated Logon Info Version 1 (data): expected=%u remains=%u",
                expected, stream.in_remain());
            throw Error(ERR_RDP_DATA_TRUNCATED);
        }

        stream.in_uni_to_ascii_str(this->UserName, this->cbUserName,
            sizeof(this->UserName));

        this->SessionId = stream.in_uint32_le();

        LOG(LOG_INFO,
            "Logon Info Version 1 (data): Domain=\"%s\" UserName=\"%s\" SessionId=%d",
            this->Domain, this->UserName, this->SessionId);
    }   // LogonInfoVersion1_Recv(Stream & stream)
};  // struct LogonInfoVersion1_Recv

/*
struct LogonInfoVersion1_Send {
    LogonInfoVersion1_Send(Stream & stream, const uint8_t * Domain,
        const uint8_t * UserName, uint32_t sessionId)
    {
        uint8_t utf16_Domain[52];
        uint8_t utf16_UserName[512];

        memset(utf16_Domain,   0, sizeof(utf16_Domain));
        uint32_t cbDomain   = UTF8toUTF16(Domain, utf16_Domain,
            sizeof(utf16_Domain)   - sizeof(uint16_t)) + 1;

        memset(utf16_UserName, 0, sizeof(utf16_UserName));
        uint32_t cbUserName = UTF8toUTF16(UserName, utf16_UserName,
            sizeof(utf16_UserName) - sizeof(uint16_t)) + 1;

        stream.out_uint32_le(cbDomain);
        stream.out_copy_bytes(utf16_Domain, sizeof(utf16_Domain));
        stream.out_uint32_le(cbUserName);
        stream.out_copy_bytes(utf16_UserName, sizeof(utf16_UserName));
        stream.out_uint32_le(sessionId);
    }
};
*/

// 2.2.10.1.1.2 Logon Info Version 2 (TS_LOGON_INFO_VERSION_2)
// ===========================================================
// TS_LOGON_INFO_VERSION_2 is a variable-length structure that contains logon
//  information intended for the client.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |            Version            |              Size             |
// +-------------------------------+-------------------------------+
// |              ...              |           SessionId           |
// +-------------------------------+-------------------------------+
// |              ...              |            cbDomain           |
// +-------------------------------+-------------------------------+
// |              ...              |           cbUserName          |
// +-------------------------------+-------------------------------+
// |              ...              |              Pad              |
// +-------------------------------+-------------------------------+
// |                              ...                              |
// +-------------------------------+-------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                   (Pad cont'd for 132 rows)                   |
// +---------------------------------------------------------------+
// |                       Domain (variable)                       |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                      UserName (variable)                      |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// Version (2 bytes): A 16-bit, unsigned integer. The logon version.

// +------------------------------+-----------+
// | Value                        | Meaning   |
// +------------------------------+-----------+
// | SAVE_SESSION_PDU_VERSION_ONE | Version 1 |
// | 0x0001                       |           |
// +------------------------------+-----------+

enum {
    SAVE_SESSION_PDU_VERSION_ONE = 1
};

// Size (4 bytes): A 32-bit, unsigned integer. The total size in bytes of
//  this structure, excluding the Domain and UserName variable-length fields.

// SessionId (4 bytes): A 32-bit, unsigned integer. The ID of the session on
//  the remote server according to the server.

// cbDomain (4 bytes): A 32-bit, unsigned integer. The size in bytes of the
//  Domain field (including the mandatory null terminator).

// cbUserName (4 bytes): A 32-bit, unsigned integer. The size in bytes of the
//  UserName field (including the mandatory null terminator).

// Pad (558 bytes): 558 bytes. Padding. Values in this field MUST be ignored.

// Domain (variable): Variable-length null-terminated Unicode string
//  containing the name of the domain to which the user is logged on. The
//  size of this field in bytes is given by the cbDomain field.

// UserName (variable): Variable-length null-terminated Unicode string
//  containing the user name which was used to log on. The size of this field
//  in bytes is given by the cbUserName field.

struct LogonInfoVersion2_Recv {
    uint16_t Version;
    uint32_t Size;
    uint32_t SessionId;
    uint32_t cbDomain;
    uint32_t cbUserName;
    uint8_t  Pad[558];
    uint8_t  Domain[512];
    uint8_t  UserName[4096];

    LogonInfoVersion2_Recv(Stream & stream) :
    Version(0),
    Size(0),
    SessionId(0),
    cbDomain(0),
    cbUserName(0) {
        memset(Pad,      0, sizeof(Pad));
        memset(Domain,   0, sizeof(Domain));
        memset(UserName, 0, sizeof(UserName));

        unsigned expected = 2 +     // Version(2)
                            4 +     // Size(4)
                            4 +     // SessionId(4)
                            4 +     // cbDomain(4)
                            4 +     // cbUserName(4)
                            558;    // Pad(558)
        if (!stream.in_check_rem(expected)) {
            LOG(LOG_ERR,
                "Truncated Logon Info Version 2 (data): expected=%u remains=%u",
                expected, stream.in_remain());
            throw Error(ERR_RDP_DATA_TRUNCATED);
        }

        this->Version    = stream.in_uint16_le();
        this->Size       = stream.in_uint32_le();
        this->SessionId  = stream.in_uint32_le();
        this->cbDomain   = stream.in_uint32_le();
        this->cbUserName = stream.in_uint32_le();

        stream.in_copy_bytes(this->Pad, sizeof(this->Pad));

        expected = this->cbDomain +
                   this->cbUserName;    // SessionId(4)
        if (!stream.in_check_rem(expected)) {
            LOG(LOG_ERR,
                "Truncated Logon Info Version 2 (data): expected=%u remains=%u",
                expected, stream.in_remain());
            throw Error(ERR_RDP_DATA_TRUNCATED);
        }

        stream.in_uni_to_ascii_str(this->Domain, this->cbDomain,
            sizeof(this->Domain));
        stream.in_uni_to_ascii_str(this->UserName, this->cbUserName,
            sizeof(this->UserName));

        LOG(LOG_INFO,
            "Logon Info Version 2 (data): Domain=\"%s\" UserName=\"%s\" SessionId=%d",
            this->Domain, this->UserName, this->SessionId);
    }   // LogonInfoVersion2_Recv(Stream & stream)
};  // struct LogonInfoVersion2_Recv

// 2.2.10.1.1.3 Plain Notify (TS_PLAIN_NOTIFY)
// ===========================================
// TS_PLAIN_NOTIFY is a fixed-length structure that contains 576 bytes of
//  padding.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                              Pad                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                   (Pad cont'd for 136 rows)                   |
// +---------------------------------------------------------------+

// Pad (576 bytes): 576 bytes. Padding. Values in this field MUST be ignored.

struct PlainNotify_Recv {
    uint8_t  Pad[558];

    PlainNotify_Recv(Stream & stream) {
        memset(Pad, 0, sizeof(Pad));

        const unsigned expected = 576;  // Pad(576)
        if (!stream.in_check_rem(expected)) {
            LOG(LOG_ERR,
                "Truncated Plain Notify (data): expected=%u remains=%u",
                expected, stream.in_remain());
            throw Error(ERR_RDP_DATA_TRUNCATED);
        }

        stream.in_copy_bytes(this->Pad, sizeof(this->Pad));
    }
};

// 2.2.10.1.1.4 Logon Info Extended (TS_LOGON_INFO_EXTENDED)
// =========================================================
// The TS_LOGON_INFO_EXTENDED structure contains RDP 5.2, 6.0, 6.1, 7.0, 7.1,
//  and 8.0 extended logon information.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |             Length            |         FieldsPresent         |
// +-------------------------------+-------------------------------+
// |              ...              |     LogonFields (variable)    |
// +-------------------------------+-------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              Pad                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                   (Pad cont'd for 134 rows)                   |
// +-------------------------------+-------------------------------+
// |              ...              |
// +-------------------------------+

// Length (2 bytes): A 16-bit, unsigned integer. The total size in bytes of
//  this structure, including the variable LogonFields field.

// FieldsPresent (4 bytes): A 32-bit, unsigned integer. The flags indicating
//  which fields are present in the LogonFields field.

// +------------------------------+--------------------------------------------+
// | Flag                         | Meaning                                    |
// +------------------------------+--------------------------------------------+
// | LOGON_EX_AUTORECONNECTCOOKIE | An auto-reconnect cookie field is present. |
// | 0x00000001                   | The LogonFields field of the associated    |
// |                              | Logon Info (section 2.2.10.1.1.4.1)        |
// |                              | structure MUST contain a Server Auto-      |
// |                              | Reconnect (section 2.2.4.2) structure.     |
// +------------------------------+--------------------------------------------+
// | LOGON_EX_LOGONERRORS         | A logon error field is present. The        |
// | 0x00000002                   | LogonFields field of the associated Logon  |
// |                              | Info MUST contain a Logon Errors Info      |
// |                              | (section 2.2.10.1.1.4.1.1) structure.      |
// +------------------------------+--------------------------------------------+

enum {
    LOGON_EX_AUTORECONNECTCOOKIE = 1,
    LOGON_EX_LOGONERRORS         = 2
};

// LogonFields (variable): Extended logon information fields encapsulated in
//  Logon Info Field (section 2.2.10.1.1.4.1) structures. The presence of an
//  information field is indicated by the flags within the FieldsPresent
//  field of the Logon Info Extended structure. The ordering of the fields is
//  implicit and is as follows:
//
//  1. Auto-reconnect cookie data
//
//  2. Logon notification data
//
//  If a field is not present, the next field which is present is read.

// Pad (570 bytes): 570 bytes. Padding. Values in this field MUST be ignored.

struct LogonInfoExtended_Recv {
    uint16_t Length;
    uint32_t FieldsPresent;

    SubStream payload;

    LogonInfoExtended_Recv(Stream & stream) :
    Length(0),
    FieldsPresent(0),
    payload(stream) {
        const unsigned expected = 2 +   // Length(2)
                                  4;    // FieldsPresent(4)
        if (!stream.in_check_rem(expected)) {
            LOG(LOG_ERR,
                "Truncated Logon Info Extended (data): expected=%u remains=%u",
                expected, stream.in_remain());
            throw Error(ERR_RDP_DATA_TRUNCATED);
        }

        this->Length        = stream.in_uint16_le();
        this->FieldsPresent = stream.in_uint32_le();

        this->payload.resize(stream, stream.in_remain());
    }
};

// 2.2.10.1.1.4.1 Logon Info Field (TS_LOGON_INFO_FIELD)
// =====================================================
// The TS_LOGON_INFO_FIELD structure is used to encapsulate extended logon
//  information field data of variable length.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                          cbFieldData                          |
// +---------------------------------------------------------------+
// |                      FieldData (variable)                     |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// cbFieldData (4 bytes): A 32-bit, unsigned integer. The size in bytes of
//  the variable-length data in the FieldData field.

// FieldData (variable): Variable-length data conforming to the structure for
//  the type given in the FieldsPresent field of the Logon Info Extended
//  (section 2.2.10.1.1.4) structure.

struct LogonInfoField_Recv {
    uint32_t cbFieldData;

    SubStream payload;

    LogonInfoField_Recv(Stream & stream) :
    cbFieldData(0),
    payload(stream) {
        const unsigned expected = 4;    // cbFieldData(4)
        if (!stream.in_check_rem(expected)) {
            LOG(LOG_ERR,
                "Truncated Logon Info Field (data): expected=%u remains=%u",
                expected, stream.in_remain());
            throw Error(ERR_RDP_DATA_TRUNCATED);
        }

        this->cbFieldData = stream.in_uint32_le();

        this->payload.resize(stream, stream.in_remain());
    }
};

// 2.2.10.1.1.4.1.1 Logon Errors Info (TS_LOGON_ERRORS_INFO)
// =========================================================
// The TS_LOGON_ERRORS_INFO structure contains information that describes a
//  logon error notification.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                     ErrorNotificationData                     |
// +---------------------------------------------------------------+
// |                     ErrorNotificationType                     |
// +---------------------------------------------------------------+

// ErrorNotificationData (4 bytes): A 32-bit, unsigned integer. An error code
//  that describes the reason for the logon notification. This value is
//  either an NTSTATUS code (see [ERRTRANS] for information on translating
//  NTSTATUS error codes to usable text strings), or one of the following
//  values.

// +--------------------------------+------------------------------------------+
// | Value                          | Meaning                                  |
// +--------------------------------+------------------------------------------+
// | LOGON_MSG_NO_PERMISSION        | The user does not have permission to log |
// | 0xFFFFFFFA                     | on.                                      |
// +--------------------------------+------------------------------------------+
// | LOGON_MSG_BUMP_OPTIONS         | Session contention UI is being           |
// | 0xFFFFFFFB                     | displayed.                               |
// +--------------------------------+------------------------------------------+
// | LOGON_MSG_SESSION_RECONNECT    | Session reconnection UI is being         |
// | 0xFFFFFFFC                     | displayed.                               |
// +--------------------------------+------------------------------------------+
// | LOGON_MSG_SESSION_TERMINATE    | The session is being terminated.         |
// | 0xFFFFFFFD                     |                                          |
// +--------------------------------+------------------------------------------+
// | LOGON_MSG_SESSION_CONTINUE     | The logon process is continuing.         |
// | 0xFFFFFFFE                     |                                          |
// +--------------------------------+------------------------------------------+

enum {
    LOGON_MSG_NO_PERMISSION     = 0xFFFFFFFA,
    LOGON_MSG_BUMP_OPTIONS      = 0xFFFFFFFB,
    LOGON_MSG_SESSION_RECONNECT = 0xFFFFFFFC,
    LOGON_MSG_SESSION_TERMINATE = 0xFFFFFFFD,
    LOGON_MSG_SESSION_CONTINUE  = 0xFFFFFFFE
};

// ErrorNotificationType (4 bytes): A 32-bit, unsigned integer. The type code
//  of the notification.

// +------------------------------+--------------------------------------------+
// | Value                        | Meaning                                    |
// +------------------------------+--------------------------------------------+
// | LOGON_FAILED_BAD_PASSWORD    | The logon process failed. The logon        |
// | 0x00000000                   | credentials which were supplied are        |
// |                              | invalid.                                   |
// +------------------------------+--------------------------------------------+
// | LOGON_FAILED_UPDATE_PASSWORD | The logon process failed. The user cannot  |
// | 0x00000001                   | continue with the logon process until the  |
// |                              | password is changed.                       |
// +------------------------------+--------------------------------------------+
// | LOGON_FAILED_OTHER           | The logon process failed. The reason for   |
// | 0x00000002                   | the failure can be deduced from the        |
// |                              | ErrorNotificationData field.               |
// +------------------------------+--------------------------------------------+
// | LOGON_WARNING                | The user received a warning during the     |
// | 0x00000003                   | logon process. The reason for the warning  |
// |                              | can be deduced from the                    |
// |                              | ErrorNotificationData field.               |
// +------------------------------+--------------------------------------------+

enum {
    LOGON_FAILED_BAD_PASSWORD    = 0x00000000,
    LOGON_FAILED_UPDATE_PASSWORD = 0x00000001,
    LOGON_FAILED_OTHER           = 0x00000002,
    LOGON_WARNING                = 0x00000003
};

struct LogonErrorsInfo_Recv {
    uint32_t ErrorNotificationData;
    uint32_t ErrorNotificationType;

    LogonErrorsInfo_Recv(Stream & stream) :
    ErrorNotificationData(0),
    ErrorNotificationType(0) {
        const unsigned expected = 4 +   // ErrorNotificationData(4)
                                  4;    // ErrorNotificationType(4)
        if (!stream.in_check_rem(expected)) {
            LOG(LOG_ERR,
                "Truncated Logon Info Field (data): expected=%u remains=%u",
                expected, stream.in_remain());
            throw Error(ERR_RDP_DATA_TRUNCATED);
        }

        this->ErrorNotificationData = stream.in_uint32_le();
        this->ErrorNotificationType = stream.in_uint32_le();

        LOG(LOG_INFO,
            "ErrorNotificationData=%s(0x%08X) \"%s\" ErrorNotificationType=%s(0x%08X) \"%s\"",
            ErrorNotificationDataToString(this->ErrorNotificationData),
            this->ErrorNotificationData,
            ErrorNotificationDataToMessage(this->ErrorNotificationData),
            ErrorNotificationTypeToString(this->ErrorNotificationType),
            this->ErrorNotificationType,
            ErrorNotificationTypeToMessage(this->ErrorNotificationType));
    }

    static const char * ErrorNotificationDataToString(
            uint32_t ErrorNotificationData) {
        switch (ErrorNotificationData) {
        case LOGON_MSG_NO_PERMISSION:
            return "LOGON_MSG_NO_PERMISSION";
        case LOGON_MSG_BUMP_OPTIONS:
            return "LOGON_MSG_BUMP_OPTIONS";
        case LOGON_MSG_SESSION_RECONNECT:
            return "LOGON_MSG_SESSION_RECONNECT";
        case LOGON_MSG_SESSION_TERMINATE:
            return "LOGON_MSG_SESSION_TERMINATE";
        case LOGON_MSG_SESSION_CONTINUE:
            return "LOGON_MSG_SESSION_CONTINUE";

        default:
            return "<Unexpected>";
        }
    }

    static const char * ErrorNotificationDataToMessage(
            uint32_t ErrorNotificationData) {
        switch (ErrorNotificationData) {
        case LOGON_MSG_NO_PERMISSION:
            return "The user does not have permission to log on.";
        case LOGON_MSG_BUMP_OPTIONS:
            return "Session contention UI is being displayed.";
        case LOGON_MSG_SESSION_RECONNECT:
            return "Session reconnection UI is being displayed.";
        case LOGON_MSG_SESSION_TERMINATE:
            return "The session is being terminated.";
        case LOGON_MSG_SESSION_CONTINUE:
            return "The logon process is continuing.";

        default:
            return "Unexpected Error Notification Data.";
        }
    }

    static const char * ErrorNotificationTypeToString(
            uint32_t ErrorNotificationType) {
        switch (ErrorNotificationType) {
        case LOGON_FAILED_BAD_PASSWORD:
            return "LOGON_FAILED_BAD_PASSWORD";
        case LOGON_FAILED_UPDATE_PASSWORD:
            return "LOGON_FAILED_UPDATE_PASSWORD";
        case LOGON_FAILED_OTHER:
            return "LOGON_FAILED_OTHER";
        case LOGON_WARNING:
            return "LOGON_WARNING";

        default:
            return "<Unexpected>";
        }
    }

    static const char * ErrorNotificationTypeToMessage(
            uint32_t ErrorNotificationType) {
        switch (ErrorNotificationType) {
        case LOGON_FAILED_BAD_PASSWORD:
            return "The logon process failed. "
                "The logon credentials which were supplied are invalid.";
        case LOGON_FAILED_UPDATE_PASSWORD:
            return "The logon process failed. "
                "The user cannot continue with the logon process until the "
                "password is changed.";
        case LOGON_FAILED_OTHER:
            return "The logon process failed. "
                "The reason for the failure can be deduced from the "
                "ErrorNotificationData field.";
        case LOGON_WARNING:
            return "The user received a warning during the logon process. "
                "The reason for the warning can be deduced from the "
                "ErrorNotificationData field.";

        default:
            return "Unexpected Error Notification Type.";
        }
    }
};  // struct LogonErrorsInfo_Recv

}   // namespace RDP

#endif  // #ifndef _REDEMPTION_CORE_RDP_SAVE_SESSION_INFO_PDU_HPP_