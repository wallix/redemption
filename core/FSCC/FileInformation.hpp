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
    Copyright (C) Wallix 2015
    Author(s): Christophe Grosjean, Raphael Zhou
*/

#ifndef _REDEMPTION_CORE_FSCC_FILEINFORMATION_HPP_
#define _REDEMPTION_CORE_FSCC_FILEINFORMATION_HPP_

#include "stream.hpp"

namespace fscc {

// [MS-FSCC] - 2.4.7 FileBasicInformation
// ======================================

// This information class is used to query or set file information.

// A FILE_BASIC_INFORMATION data element, defined as follows, is returned by
//  the server or provided by the client.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                          CreationTime                         |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                         LastAccessTime                        |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                         LastWriteTime                         |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                           ChangeTime                          |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                         FileAttributes                        |
// +---------------------------------------------------------------+
// |                            Reserved                           |
// +---------------------------------------------------------------+

// CreationTime (8 bytes): The time when the file was created; see section
//  2.1.1. A valid time for this field is an integer greater than or equal to
//  0. When setting file attributes, a value of 0 indicates to the server
//  that it MUST NOT change this attribute. When setting file attributes, a
//  value of -1 indicates to the server that it MUST NOT change this
//  attribute for all subsequent operations on the same file handle. This
//  field MUST NOT be set to a value less than -1.

// LastAccessTime (8 bytes): The last time the file was accessed; see section
//  2.1.1. A valid time for this field is an integer greater than or equal to
//  0. When setting file attributes, a value of 0 indicates to the server
//  that it MUST NOT change this attribute. When setting file attributes, a
//  value of -1 indicates to the server that it MUST NOT change this
//  attribute for all subsequent operations on the same file handle. This
//  field MUST NOT be set to a value less than -1.<89>

// LastWriteTime (8 bytes): The last time information was written to the
//  file; see section 2.1.1. A valid time for this field is an integer
//  greater than or equal to 0. When setting file attributes, a value of 0
//  indicates to the server that it MUST NOT change this attribute. When
//  setting file attributes, a value of -1 indicates to the server that it
//  MUST NOT change this attribute for all subsequent operations on the same
//  file handle. This field MUST NOT be set to a value less than -1.<90>

// ChangeTime (8 bytes): The last time the file was changed; see section
//  2.1.1. A valid time for this field is an integer greater than or equal to
//  0. When setting file attributes, a value of 0 indicates to the server
//  that it MUST NOT change this attribute. When setting file attributes, a
//  value of -1 indicates to the server that it MUST NOT change this
//  attribute for all subsequent operations on the same file handle. This
//  field MUST NOT be set to a value less than -1.<91>

// FileAttributes (4 bytes): A 32-bit unsigned integer that contains the file
//  attributes. Valid file attributes are specified in section 2.6.

// Reserved (4 bytes): A 32-bit field. This field is reserved. This field can
//  be set to any value, and MUST be ignored.

// This operation returns a status code, as specified in [MS-ERREF] section
//  2.3. The status code returned directly by the function that processes
//  this file information class MUST be STATUS_SUCCESS or one of the
//  following.

//  +-----------------------------+--------------------------------------------+
//  | Error code                  | Meaning                                    |
//  +-----------------------------+--------------------------------------------+
//  | STATUS_INFO_LENGTH_MISMATCH | The specified information record length    |
//  | 0xC0000004                  | does not match the length that is required |
//  |                             | for the specified information class.       |
//  +-----------------------------+--------------------------------------------+
//  | STATUS_ACCESS_DENIED        | The handle was not opened to read file     |
//  | 0xC0000022                  | data or file attributes.                   |
//  +-----------------------------+--------------------------------------------+

class FileBasicInformation {
    uint64_t CreationTime   = 0;
    uint64_t LastAccessTime = 0;
    uint64_t LastWriteTime  = 0;
    uint64_t ChangeTime     = 0;
    uint32_t FileAttributes = 0;

public:
    FileBasicInformation() = default;

    FileBasicInformation(uint64_t CreationTime, uint64_t LastAccessTime,
                         uint64_t LastWriteTime, uint64_t ChangeTime,
                         uint32_t FileAttributes)
    : CreationTime(CreationTime)
    , LastAccessTime(LastAccessTime)
    , LastWriteTime(LastWriteTime)
    , ChangeTime(ChangeTime)
    , FileAttributes(FileAttributes) {}

    inline void emit(Stream & stream) const {
        stream.out_uint64_le(this->CreationTime);
        stream.out_uint64_le(this->LastAccessTime);
        stream.out_uint64_le(this->LastWriteTime);
        stream.out_uint64_le(this->ChangeTime);

        stream.out_uint32_le(this->FileAttributes);

        // Reserved(4), MUST NOT be transmitted.
    }

    inline void receive(Stream & stream) {
        {
            const unsigned expected = 36;   // CreationTime(8) + LastAccessTime(8) +
                                            //     LastWriteTime(8) + ChangeTime(8) +
                                            //     FileAttributes(4)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated FileBasicInformation: expected=%u remains=%u",
                    expected, stream.in_remain());
                throw Error(ERR_FSCC_DATA_TRUNCATED);
            }
        }

        this->CreationTime   = stream.in_uint64_le();
        this->LastAccessTime = stream.in_uint64_le();
        this->LastWriteTime  = stream.in_uint64_le();
        this->ChangeTime     = stream.in_uint64_le();

        this->FileAttributes = stream.in_uint32_le();

        // Reserved(4), MUST NOT be transmitted.
    }

    inline static size_t size() {
        return 36;  /* CreationTime(8) + LastAccessTime(8) + LastWriteTime(8) + ChangeTime(8) + FileAttributes(4) */
    }

private:
    size_t str(char * buffer, size_t size) const {
        size_t length = ::snprintf(buffer, size,
            "FileBasicInformation: CreationTime=%" PRIu64 " LastAccessTime=%" PRIu64
                " LastWriteTime=%" PRIu64 " ChangeTime=%" PRIu64 " FileAttributes=0x%X",
            this->CreationTime, this->LastAccessTime, this->LastWriteTime,
            this->ChangeTime, this->FileAttributes);
        return ((length < size) ? length : size - 1);
    }

public:
    inline void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, buffer);
    }
};

// [MS-FSCC] - 2.6 File Attributes
// ===============================

// The following attributes are defined for files and directories. They can
//  be used in any combination unless noted in the description of the
//  attribute's meaning. There is no file attribute with the value 0x00000000
//  because a value of 0x00000000 in the FileAttributes field means that the
//  file attributes for this file MUST NOT be changed when setting basic
//  information for the file.

//  +------------------------------------+-------------------------------------+
//  | Value                              | Meaning                             |
//  +------------------------------------+-------------------------------------+
//  | FILE_ATTRIBUTE_ARCHIVE             | A file or directory that requires   |
//  | 0x00000020                         | to be archived. Applications use    |
//  |                                    | this attribute to mark files for    |
//  |                                    | backup or removal.                  |
//  +------------------------------------+-------------------------------------+
//  | FILE_ATTRIBUTE_COMPRESSED          | A file or directory that is         |
//  | 0x00000800                         | compressed. For a file, all of the  |
//  |                                    | data in the file is compressed. For |
//  |                                    | a directory, compression is the     |
//  |                                    | default for newly created files and |
//  |                                    | subdirectories.                     |
//  +------------------------------------+-------------------------------------+
//  | FILE_ATTRIBUTE_DIRECTORY           | This item is a directory.           |
//  | 0x00000010                         |                                     |
//  +------------------------------------+-------------------------------------+
//  | FILE_ATTRIBUTE_ENCRYPTED           | A file or directory that is         |
//  | 0x00004000                         | encrypted. For a file, all data     |
//  |                                    | streams in the file are encrypted.  |
//  |                                    | For a directory, encryption is the  |
//  |                                    | default for newly created files and |
//  |                                    | subdirectories.                     |
//  +------------------------------------+-------------------------------------+
//  | FILE_ATTRIBUTE_HIDDEN              | A file or directory that is hidden. |
//  | 0x00000002                         | Files and directories marked with   |
//  |                                    | this attribute do not appear in an  |
//  |                                    | ordinary directory listing.         |
//  +------------------------------------+-------------------------------------+
//  | FILE_ATTRIBUTE_NORMAL              | A file that does not have other     |
//  | 0x00000080                         | attributes set. This flag is used   |
//  |                                    | to clear all other flags by         |
//  |                                    | specifying it with no other flags   |
//  |                                    | set.                                |
//  |                                    | This flag MUST be ignored if other  |
//  |                                    | flags are set.<157>                 |
//  +------------------------------------+-------------------------------------+
//  | FILE_ATTRIBUTE_NOT_CONTENT_INDEXED | A file or directory that is not     |
//  | 0x00002000                         | indexed by the content indexing     |
//  |                                    | service.                            |
//  +------------------------------------+-------------------------------------+
//  | FILE_ATTRIBUTE_OFFLINE             | The data in this file is not        |
//  | 0x00001000                         | available immediately. This         |
//  |                                    | attribute indicates that the file   |
//  |                                    | data is physically moved to offline |
//  |                                    | storage. This attribute is used by  |
//  |                                    | Remote Storage, which is            |
//  |                                    | hierarchical storage management     |
//  |                                    | software.                           |
//  +------------------------------------+-------------------------------------+
//  | FILE_ATTRIBUTE_READONLY            | A file or directory that is read-   |
//  | 0x00000001                         |only. For a file, applications can   |
//  |                                    | read the file but cannot write to   |
//  |                                    | it or delete it. For a directory,   |
//  |                                    | applications cannot delete it, but  |
//  |                                    | applications can create and delete  |
//  |                                    | files from that directory.          |
//  +------------------------------------+-------------------------------------+
//  | FILE_ATTRIBUTE_REPARSE_POINT       | A file or directory that has an     |
//  | 0x00000400                         | associated reparse point.           |
//  +------------------------------------+-------------------------------------+
//  | FILE_ATTRIBUTE_SPARSE_FILE         | A file that is a sparse file.       |
//  | 0x00000200                         |                                     |
//  +------------------------------------+-------------------------------------+
//  | FILE_ATTRIBUTE_SYSTEM              | A file or directory that the        |
//  | 0x00000004                         | operating system uses a part of or  |
//  |                                    | uses exclusively.                   |
//  +------------------------------------+-------------------------------------+
//  | FILE_ATTRIBUTE_TEMPORARY           | A file that is being used for       |
//  | 0x00000100                         | temporary storage. The operating    |
//  |                                    | system may choose to store this     |
//  |                                    | file's data in memory rather than   |
//  |                                    | on mass storage, writing the data   |
//  |                                    | to mass storage only if data        |
//  |                                    | remains in the file when the file   |
//  |                                    | is closed.                          |
//  +------------------------------------+-------------------------------------+
//  | FILE_ATTRIBUTE_INTEGRITY_STREAM    | A file or directory that is         |
//  | 0x00008000                         | configured with integrity support.  |
//  |                                    | For a file, all data streams in the |
//  |                                    | file have integrity support. For a  |
//  |                                    | directory, integrity support is the |
//  |                                    | default for newly created files and |
//  |                                    | subdirectories, unless the caller   |
//  |                                    | specifies otherwise.<158>           |
//  +------------------------------------+-------------------------------------+
//  | FILE_ATTRIBUTE_NO_SCRUB_DATA       | A file or directory that is         |
//  | 0x00020000                         | configured to be excluded from the  |
//  |                                    | data integrity scan. For a          |
//  |                                    | directory configured with           |
//  |                                    | FILE_ATTRIBUTE_NO_SCRUB_DATA, the   |
//  |                                    | default for newly created files and |
//  |                                    | subdirectories is to inherit the    |
//  |                                    | FILE_ATTRIBUTE_NO_SCRUB_DATA        |
//  |                                    | attribute.<159>                     |
//  +------------------------------------+-------------------------------------+

enum {
      FILE_ATTRIBUTE_ARCHIVE             = 0x00000020
    , FILE_ATTRIBUTE_COMPRESSED          = 0x00000800
    , FILE_ATTRIBUTE_DIRECTORY           = 0x00000010
    , FILE_ATTRIBUTE_ENCRYPTED           = 0x00004000
    , FILE_ATTRIBUTE_HIDDEN              = 0x00000002
    , FILE_ATTRIBUTE_NORMAL              = 0x00000080
    , FILE_ATTRIBUTE_NOT_CONTENT_INDEXED = 0x00002000
    , FILE_ATTRIBUTE_OFFLINE             = 0x00001000
    , FILE_ATTRIBUTE_READONLY            = 0x00000001
    , FILE_ATTRIBUTE_REPARSE_POINT       = 0x00000400
    , FILE_ATTRIBUTE_SPARSE_FILE         = 0x00000200
    , FILE_ATTRIBUTE_SYSTEM              = 0x00000004
    , FILE_ATTRIBUTE_TEMPORARY           = 0x00000100
    , FILE_ATTRIBUTE_INTEGRITY_STREAM    = 0x00008000
    , FILE_ATTRIBUTE_NO_SCRUB_DATA       = 0x00020000
};

}   // namespace fscc

#endif  // #ifndef _REDEMPTION_CORE_FSCC_FILEINFORMATION_HPP_
