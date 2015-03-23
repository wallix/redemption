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
};  // FileBasicInformation

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

// [MS-FSCC] - 2.4.8 FileBothDirectoryInformation
// ==============================================

// This information class is used in directory enumeration to return detailed
//  information about the contents of a directory.

// This information class returns a list that contains a
//  FILE_BOTH_DIR_INFORMATION data element for each file or directory within
//  the target directory. This list MUST reflect the presence of a
//  subdirectory named "." (synonymous with the target directory itself)
//  within the target directory and one named ".." (synonymous with the
//  parent directory of the target directory). For more details, see section
//  2.1.5.1.

// This information class differs from FileDirectoryInformation (section
//  2.4.10) in that it includes short names in the returns list.

// When multiple FILE_BOTH_DIR_INFORMATION data elements are present in the
//  buffer, each MUST be aligned on an 8-byte boundary. Any bytes inserted
//  for alignment SHOULD be set to zero, and the receiver MUST ignore them.
//  No padding is required following the last data element.

// A FILE_BOTH_DIR_INFORMATION data element is as follows.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                        NextEntryOffset                        |
// +---------------------------------------------------------------+
// |                           FileIndex                           |
// +---------------------------------------------------------------+
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
// |                           EndOfFile                           |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                         AllocationSize                        |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                         FileAttributes                        |
// +---------------------------------------------------------------+
// |                         FileNameLength                        |
// +---------------------------------------------------------------+
// |                             EaSize                            |
// +---------------+---------------+-------------------------------+
// |ShortNameLength|    Reserved   |           ShortName           |
// +---------------+---------------+-------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                              ...                              |
// +-------------------------------+-------------------------------+
// |              ...              |      FileName (variable)      |
// +-------------------------------+-------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// NextEntryOffset (4 bytes): A 32-bit unsigned integer that contains the
//  byte offset from the beginning of this entry, at which the next
//  FILE_BOTH_DIR_INFORMATION entry is located, if multiple entries are
//  present in a buffer. This member is zero if no other entries follow this
//  one. An implementation MUST use this value to determine the location of
//  the next entry (if multiple entries are present in a buffer).

// FileIndex (4 bytes): A 32-bit unsigned integer that contains the byte
//  offset of the file within the parent directory. For file systems in which
//  the position of a file within the parent directory is not fixed and can
//  be changed at any time to maintain sort order, this field SHOULD be set
//  to 0x00000000 and MUST be ignored.<92>

// CreationTime (8 bytes): The time when the file was created; see section
//  2.1.1. This value MUST be greater than or equal to 0.

// LastAccessTime (8 bytes): The last time the file was accessed; see section
//  2.1.1. This value MUST be greater than or equal to 0.

// LastWriteTime (8 bytes): The last time information was written to the
//  file; see section 2.1.1. This value MUST be greater than or equal to 0.

// ChangeTime (8 bytes): The last time the file was changed; see section
//  2.1.1. This value MUST be greater than or equal to 0.

// EndOfFile (8 bytes): A 64-bit signed integer that contains the absolute
//  new end-of-file position as a byte offset from the start of the file.
//  EndOfFile specifies the offset to the byte immediately following the last
//  valid byte in the file. Because this value is zero-based, it actually
//  refers to the first free byte in the file. That is, it is the offset from
//  the beginning of the file at which new bytes appended to the file will be
//  written. The value of this field MUST be greater than or equal to 0.

// AllocationSize (8 bytes): A 64-bit signed integer that contains the file
//  allocation size, in bytes. The value of this field MUST be an integer
//  multiple of the cluster size.

// FileAttributes (4 bytes): A 32-bit unsigned integer that contains the file
//  attributes. Valid file attributes are specified in section 2.6.

// FileNameLength (4 bytes): A 32-bit unsigned integer that specifies the
//  length, in bytes, of the file name contained within the FileName member.

// EaSize (4 bytes): A 32-bit unsigned integer that contains the combined
//  length, in bytes, of the extended attributes (EA) for the file.

// ShortNameLength (1 byte): An 8-bit signed integer that specifies the
//  length, in bytes, of the file name contained in the ShortName member.
//  This value MUST be greater than or equal to 0.

// Reserved (1 byte): Reserved for alignment. This field can contain any
//  value and MUST be ignored.

// ShortName (24 bytes): A sequence of Unicode characters containing the
//  short (8.3) file name. When working with this field, use ShortNameLength
//  to determine the length of the file name rather than assuming the
//  presence of a trailing null delimiter.

// FileName (variable): A sequence of Unicode characters containing the file
//  name. When working with this field, use FileNameLength to determine the
//  length of the file name rather than assuming the presence of a trailing
//  null delimiter. Dot directory names are valid for this field. For more
//  details, see section 2.1.5.1.

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

class FileBothDirectoryInformation {
    uint32_t NextEntryOffset = 0;
    uint32_t FileIndex       = 0;
    uint64_t CreationTime    = 0;
    uint64_t LastAccessTime  = 0;
    uint64_t LastWriteTime   = 0;
    uint64_t ChangeTime      = 0;
    int64_t  EndOfFile       = 0;
    int64_t  AllocationSize  = 0;
    uint32_t FileAttributes  = 0;
    uint32_t EaSize          = 0;

    std::string short_name;
    std::string file_name;

public:
    FileBothDirectoryInformation() = default;

    FileBothDirectoryInformation(uint64_t CreationTime, uint64_t LastAccessTime,
                         uint64_t LastWriteTime, uint64_t ChangeTime,
                         int64_t EndOfFile, int64_t AllocationSize,
                         uint32_t FileAttributes, const char * file_name)
    : CreationTime(CreationTime)
    , LastAccessTime(LastAccessTime)
    , LastWriteTime(LastWriteTime)
    , ChangeTime(ChangeTime)
    , EndOfFile(EndOfFile)
    , AllocationSize(AllocationSize)
    , FileAttributes(FileAttributes)
    , file_name(file_name) {}

    inline void emit(Stream & stream) const {
        stream.out_uint32_le(this->NextEntryOffset);
        stream.out_uint32_le(this->FileIndex);

        stream.out_uint64_le(this->CreationTime);
        stream.out_uint64_le(this->LastAccessTime);
        stream.out_uint64_le(this->LastWriteTime);
        stream.out_uint64_le(this->ChangeTime);

        stream.out_sint64_le(this->EndOfFile);
        stream.out_sint64_le(this->AllocationSize);

        stream.out_uint32_le(this->FileAttributes);

        // The null-terminator is included.
        const size_t maximum_length_of_FileName_in_bytes = (this->file_name.length() + 1) * 2;

        uint8_t * const FileName_unicode_data = static_cast<uint8_t *>(::alloca(
                    maximum_length_of_FileName_in_bytes));
        size_t size_of_FileName_unicode_data = ::UTF8toUTF16(
            reinterpret_cast<const uint8_t *>(this->file_name.c_str()), FileName_unicode_data,
            maximum_length_of_FileName_in_bytes);
        // Writes null terminator.
        FileName_unicode_data[size_of_FileName_unicode_data    ] =
        FileName_unicode_data[size_of_FileName_unicode_data + 1] = 0;
        size_of_FileName_unicode_data += 2;

        stream.out_uint32_le(size_of_FileName_unicode_data);

        stream.out_uint32_le(this->EaSize);

        const size_t maximum_length_of_ShortName_in_bytes = (this->short_name.length() + 1) * 2;

        uint8_t * const ShortName_unicode_data = static_cast<uint8_t *>(::alloca(
                    maximum_length_of_ShortName_in_bytes));
        size_t size_of_ShortName_unicode_data = ::UTF8toUTF16(
            reinterpret_cast<const uint8_t *>(this->short_name.c_str()), ShortName_unicode_data,
            maximum_length_of_ShortName_in_bytes);
        if (size_of_ShortName_unicode_data > 0) {
            // Writes null terminator.
            ShortName_unicode_data[size_of_ShortName_unicode_data    ] =
            ShortName_unicode_data[size_of_ShortName_unicode_data + 1] = 0;
            size_of_ShortName_unicode_data += 2;

            REDASSERT(size_of_ShortName_unicode_data <= 24 /* ShortName(24) */);
        }

        stream.out_sint8(size_of_ShortName_unicode_data);

        // Reserved(4), MUST NOT be transmitted.

        stream.out_copy_bytes(ShortName_unicode_data, size_of_ShortName_unicode_data);
        if (size_of_ShortName_unicode_data < 24  /* ShortName(24) */) {
            stream.out_clear_bytes(24 /* ShortName(24) */ - size_of_ShortName_unicode_data);
        }

        stream.out_copy_bytes(FileName_unicode_data, size_of_FileName_unicode_data);
    }

    inline void receive(Stream & stream) {
        {
            const unsigned expected = 93;   // NextEntryOffset(4) + FileIndex(4) +
                                            //     CreationTime(8) + LastAccessTime(8) +
                                            //     LastWriteTime(8) + ChangeTime(8) +
                                            //     EndOfFile(8) + AllocationSize(8) +
                                            //     FileAttributes(4) + FileNameLength(4) +
                                            //     EaSize(4) + ShortNameLength(1) +
                                            //     ShortName(24)
            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated FileBothDirectoryInformation (0): expected=%u remains=%u",
                    expected, stream.in_remain());
                throw Error(ERR_FSCC_DATA_TRUNCATED);
            }
        }

        this->NextEntryOffset = stream.in_uint32_le();
        this->FileIndex       = stream.in_uint32_le();
        this->CreationTime    = stream.in_uint64_le();
        this->LastAccessTime  = stream.in_uint64_le();
        this->LastWriteTime   = stream.in_uint64_le();
        this->ChangeTime      = stream.in_uint64_le();
        this->EndOfFile       = stream.in_sint64_le();
        this->AllocationSize  = stream.in_sint64_le();
        this->FileAttributes  = stream.in_uint32_le();

        const uint32_t FileNameLength = stream.in_uint32_le();

        this->EaSize = stream.in_uint32_le();

        const int8_t ShortNameLength = stream.in_sint8();

        // Reserved(1), MUST NOT be transmitted.

        uint8_t ShortName[24];

        stream.in_copy_bytes(ShortName, sizeof(ShortName));

        const size_t maximum_length_of_utf8_character_in_bytes = 4;

        const size_t size_of_ShortName_utf8_string =
            ShortNameLength / 2 * maximum_length_of_utf8_character_in_bytes + 1;
        uint8_t * const ShortName_utf8_string = static_cast<uint8_t *>(
            ::alloca(size_of_ShortName_utf8_string));
        const size_t length_of_ShortName_utf8_string = ::UTF16toUTF8(
            ShortName, ShortNameLength / 2, ShortName_utf8_string, size_of_ShortName_utf8_string);
        this->short_name.assign(::char_ptr_cast(ShortName_utf8_string),
            length_of_ShortName_utf8_string);

        {
            const unsigned expected = FileNameLength;   // FileName(variable)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated FileBothDirectoryInformation (1): expected=%u remains=%u",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }

        uint8_t * const FileName_unicode_data = static_cast<uint8_t *>(::alloca(FileNameLength));

        stream.in_copy_bytes(FileName_unicode_data, FileNameLength);

        const size_t size_of_FileName_utf8_string =
            FileNameLength / 2 * maximum_length_of_utf8_character_in_bytes + 1;
        uint8_t * const FileName_utf8_string = static_cast<uint8_t *>(
            ::alloca(size_of_FileName_utf8_string));
        const size_t length_of_FileName_utf8_string = ::UTF16toUTF8(
            FileName_unicode_data, FileNameLength / 2, FileName_utf8_string, size_of_FileName_utf8_string);
        this->file_name.assign(::char_ptr_cast(FileName_utf8_string),
            length_of_FileName_utf8_string);
    }

    inline size_t size() const {
        size_t size = 93;   // NextEntryOffset(4) + FileIndex(4) +
                            //     CreationTime(8) + LastAccessTime(8) +
                            //     LastWriteTime(8) + ChangeTime(8) +
                            //     EndOfFile(8) + AllocationSize(8) +
                            //     FileAttributes(4) + FileNameLength(4) +
                            //     EaSize(4) + ShortNameLength(1) +
                            //     ShortName(24)

        // Reserved(1), MUST NOT be transmitted.

        // The null-terminator is included.
        const size_t maximum_length_of_FileName_in_bytes = (this->file_name.length() + 1) * 2;

        uint8_t * const unicode_data = static_cast<uint8_t *>(::alloca(
                    maximum_length_of_FileName_in_bytes));
        size_t size_of_unicode_data = ::UTF8toUTF16(
            reinterpret_cast<const uint8_t *>(this->file_name.c_str()), unicode_data,
            maximum_length_of_FileName_in_bytes);
        // Writes null terminator.
        unicode_data[size_of_unicode_data    ] =
        unicode_data[size_of_unicode_data + 1] = 0;
        size_of_unicode_data += 2;

        return size + size_of_unicode_data;
    }

private:
    size_t str(char * buffer, size_t size) const {
        size_t length = ::snprintf(buffer, size,
            "FileBothDirectoryInformation: NextEntryOffset=%u FileIndex=%u CreationTime=%" PRIu64
                " LastAccessTime=%" PRIu64 " LastWriteTime=%" PRIu64 " ChangeTime=%" PRIu64
                " EndOfFile=%" PRId64 " AllocationSize=%" PRId64 " FileAttributes=0x%X "
                "EaSize=%u ShortName=\"%s\" FileName=\"%s\"",
            this->NextEntryOffset, this->FileIndex,
            this->CreationTime, this->LastAccessTime, this->LastWriteTime,
            this->ChangeTime, this->EndOfFile, this->AllocationSize, this->FileAttributes,
            this->EaSize, this->short_name.c_str(), this->file_name.c_str());
        return ((length < size) ? length : size - 1);
    }

public:
    inline void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, buffer);
    }
};  // FileBothDirectoryInformation

// [MS-FSCC] - 2.4.38 FileStandardInformation
// ==========================================

// This information class is used to query file information.

// A FILE_STANDARD_INFORMATION data element, defined as follows, is returned
//  by the server.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                         AllocationSize                        |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                           EndOfFile                           |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                         NumberOfLinks                         |
// +---------------+---------------+-------------------------------+
// | DeletePending |   Directory   |            Reserved           |
// +---------------+---------------+-------------------------------+

// AllocationSize (8 bytes): A 64-bit signed integer that contains the file
//  allocation size, in bytes. The value of this field MUST be an integer
//  multiple of the cluster size.

// EndOfFile (8 bytes): A 64-bit signed integer that contains the absolute
//  end-of-file position as a byte offset from the start of the file.
//  EndOfFile specifies the offset to the byte immediately following the last
//  valid byte in the file. Because this value is zero-based, it actually
//  refers to the first free byte in the file. That is, it is the offset from
//  the beginning of the file at which new bytes appended to the file will be
//  written. The value of this field MUST be greater than or equal to 0.

// NumberOfLinks (4 bytes): A 32-bit unsigned integer that contains the
//  number of non-deleted links to this file.

// DeletePending (1 byte): A Boolean (section 2.1.8) value. Set to TRUE to
//  indicate that a file deletion has been requested; set to FALSE otherwise.

// Directory (1 byte): A Boolean (section 2.1.8) value. Set to TRUE to
//  indicate that the file is a directory; set to FALSE otherwise.

// Reserved (2 bytes): A 16-bit field. This field is reserved. This field can
//  be set to any value, and MUST be ignored.

// This operation returns a status code, as specified in [MS-ERREF] section
//  2.3. The status code returned directly by the function that processes
//  this file information class MUST be STATUS_SUCCESS or one of the
//  following.

//  +-----------------------------+-----------------------------------------+
//  | Error code                  | Meaning                                 |
//  +-----------------------------+-----------------------------------------+
//  | STATUS_INFO_LENGTH_MISMATCH | The specified information record length |
//  | 0xC0000004                  | does not match the length that is       |
//  |                             | required for the specified information  |
//  |                             | class.                                  |
//  +-----------------------------+-----------------------------------------+

class FileStandardInformation {
    int64_t  AllocationSize = 0;
    int64_t  EndOfFile      = 0;
    uint32_t NumberOfLinks  = 0;
    uint8_t  DeletePending  = 0;
    uint8_t  Directory      = 0;

public:
    FileStandardInformation() = default;

    FileStandardInformation(int64_t AllocationSize, int64_t EndOfFile,
                            uint32_t NumberOfLinks, uint8_t DeletePending,
                            uint8_t Directory)
    : AllocationSize(AllocationSize)
    , EndOfFile(EndOfFile)
    , NumberOfLinks(NumberOfLinks)
    , DeletePending(DeletePending)
    , Directory(Directory) {}

    inline void emit(Stream & stream) const {
        stream.out_sint64_le(this->AllocationSize);
        stream.out_sint64_le(this->EndOfFile);

        stream.out_uint32_le(this->NumberOfLinks);

        stream.out_uint8(this->DeletePending);
        stream.out_uint8(this->Directory);

        // Reserved(2), MUST NOT be transmitted.
    }

    inline void receive(Stream & stream) {
        {
            const unsigned expected = 22;   // AllocationSize(8) + EndOfFile(8) +
                                            //     NumberOfLinks(4) + DeletePending(1) +
                                            //     Directory(1)
            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated FileStandardInformation: expected=%u remains=%u",
                    expected, stream.in_remain());
                throw Error(ERR_FSCC_DATA_TRUNCATED);
            }
        }

        this->AllocationSize = stream.in_sint64_le();
        this->EndOfFile      = stream.in_sint64_le();
        this->NumberOfLinks  = stream.in_uint32_le();
        this->DeletePending  = stream.in_uint8();
        this->Directory      = stream.in_uint8();
    }

    inline static size_t size() {
        return 22;  // AllocationSize(8) + EndOfFile(8) +
                    //     NumberOfLinks(4) + DeletePending(1) +
                    //     Directory(1)
    }

private:
    size_t str(char * buffer, size_t size) const {
        size_t length = ::snprintf(buffer, size,
            "FileBothDirectoryInformation: AllocationSize=%" PRId64
                " EndOfFile=%" PRId64 " NumberOfLinks=%u "
                "DeletePending=%u Directory=%u",
            this->AllocationSize, this->EndOfFile, this->NumberOfLinks,
            this->DeletePending, this->Directory);
        return ((length < size) ? length : size - 1);
    }

public:
    inline void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, buffer);
    }
};  // FileStandardInformation

// [MS-FSCC] - 2.5.9 FileFsVolumeInformation
// =========================================

// This information class is used to query information on a volume on which a
//  file system is mounted. A FILE_FS_VOLUME_INFORMATION data element,
//  defined as follows, is returned by the server.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                       VolumeCreationTime                      |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                       VolumeSerialNumber                      |
// +---------------------------------------------------------------+
// |                       VolumeLabelLength                       |
// +---------------+---------------+-------------------------------+
// |SupportsObjects|    Reserved   |     VolumeLabel (variable)    |
// +---------------+---------------+-------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// VolumeCreationTime (8 bytes): The time when the volume was created; see
//  section 2.1.1. The value of this field MUST be greater than or equal to
//  0.

// VolumeSerialNumber (4 bytes): A 32-bit unsigned integer that contains the
//  serial number of the volume. The serial number is an opaque value
//  generated by the file system at format time, and is not necessarily
//  related to any hardware serial number for the device on which the file
//  system is located. No specific format or content of this field is
//  required for protocol interoperation. This value is not required to be
//  unique.

// VolumeLabelLength (4 bytes): A 32-bit unsigned integer that contains the
//  length, in bytes, including the trailing null, if present, of the name of
//  the volume.<154>

// SupportsObjects (1 byte): A Boolean (section 2.1.8) value. Set to TRUE if
//  the file system supports object-oriented file system objects; set to
//  FALSE otherwise.<155>

// Reserved (1 byte): An 8-bit field. This field is reserved. This field MUST
//  be set to zero and MUST be ignored.

// VolumeLabel (variable): A variable-length Unicode field containing the
//  name of the volume. The content of this field can be a null-terminated
//  string or can be a string padded with the space character to be
//  VolumeLabelLength bytes long.

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
//  | STATUS_BUFFER_OVERFLOW      | The output buffer was filled before all of |
//  | 0x80000005                  | the volume information could be returned;  |
//  |                             | only a portion of the VolumeLabel field is |
//  |                             | returned.                                  |
//  +-----------------------------+--------------------------------------------+

class FileFsVolumeInformation {
    uint64_t VolumeCreationTime;
    uint32_t VolumeSerialNumber;
    uint8_t  SupportsObjects;

    std::string volume_label;

public:
    FileFsVolumeInformation() = default;

    FileFsVolumeInformation(uint64_t VolumeCreationTime, uint32_t VolumeSerialNumber,
                         uint8_t SupportsObjects, const char * volume_label)
    : VolumeCreationTime(VolumeCreationTime)
    , VolumeSerialNumber(VolumeSerialNumber)
    , SupportsObjects(SupportsObjects)
    , volume_label(volume_label) {}

    inline void emit(Stream & stream) const {
        stream.out_uint64_le(this->VolumeCreationTime);
        stream.out_uint32_le(this->VolumeSerialNumber);

        // The null-terminator is included.
        const size_t maximum_length_of_VolumeLabel_in_bytes = (this->volume_label.length() + 1) * 2;

        uint8_t * const unicode_data = static_cast<uint8_t *>(::alloca(
                    maximum_length_of_VolumeLabel_in_bytes));
        size_t size_of_unicode_data = ::UTF8toUTF16(
            reinterpret_cast<const uint8_t *>(this->volume_label.c_str()), unicode_data,
            maximum_length_of_VolumeLabel_in_bytes);
        // Writes null terminator.
        unicode_data[size_of_unicode_data    ] =
        unicode_data[size_of_unicode_data + 1] = 0;
        size_of_unicode_data += 2;

        stream.out_uint32_le(size_of_unicode_data); // VolumeLabelLength(4)

        stream.out_uint8(this->SupportsObjects);

        // Reserved(1), MUST NOT be transmitted.

        stream.out_copy_bytes(unicode_data, size_of_unicode_data);
    }

    inline void receive(Stream & stream) {
        {
            const unsigned expected = 17;   // VolumeCreationTime(8) + VolumeSerialNumber(4) +
                                            //     VolumeLabelLength(4) + SupportsObjects(1)
            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated FileFsVolumeInformation (0): expected=%u remains=%u",
                    expected, stream.in_remain());
                throw Error(ERR_FSCC_DATA_TRUNCATED);
            }
        }

        this->VolumeCreationTime = stream.in_uint64_le();
        this->VolumeSerialNumber = stream.in_uint32_le();

        const uint32_t VolumeLabelLength = stream.in_uint32_le();

        this->SupportsObjects  = stream.in_uint8();

        // Reserved(1), MUST NOT be transmitted.

        {
            const unsigned expected = VolumeLabelLength;    // VolumeLabel(variable)

            if (!stream.in_check_rem(expected)) {
                LOG(LOG_ERR,
                    "Truncated FileFsVolumeInformation (1): expected=%u remains=%u",
                    expected, stream.in_remain());
                throw Error(ERR_RDPDR_PDU_TRUNCATED);
            }
        }

        uint8_t * const unicode_data = static_cast<uint8_t *>(::alloca(VolumeLabelLength));

        stream.in_copy_bytes(unicode_data, VolumeLabelLength);

        const size_t maximum_length_of_utf8_character_in_bytes = 4;

        const size_t size_of_utf8_string =
            VolumeLabelLength / 2 * maximum_length_of_utf8_character_in_bytes + 1;
        uint8_t * const utf8_string = static_cast<uint8_t *>(
            ::alloca(size_of_utf8_string));
        const size_t length_of_utf8_string = ::UTF16toUTF8(
            unicode_data, VolumeLabelLength / 2, utf8_string, size_of_utf8_string);

        for (uint8_t * c = utf8_string + length_of_utf8_string - 1;
             (c >= utf8_string) && ((*c) == ' '); c--) {
            *c = '\0';
        }

        this->volume_label = ::char_ptr_cast(utf8_string);
    }

    inline size_t size() const {
        size_t size = 17;   // VolumeCreationTime(8) + VolumeSerialNumber(4) +
                            //     VolumeLabelLength(4) + SupportsObjects(1)

        // Reserved(1), MUST NOT be transmitted.

        // The null-terminator is included.
        const size_t maximum_length_of_VolumeLabel_in_bytes = (this->volume_label.length() + 1) * 2;

        uint8_t * const unicode_data = static_cast<uint8_t *>(::alloca(
                    maximum_length_of_VolumeLabel_in_bytes));
        size_t size_of_unicode_data = ::UTF8toUTF16(
            reinterpret_cast<const uint8_t *>(this->volume_label.c_str()), unicode_data,
            maximum_length_of_VolumeLabel_in_bytes);
        // Writes null terminator.
        unicode_data[size_of_unicode_data    ] =
        unicode_data[size_of_unicode_data + 1] = 0;
        size_of_unicode_data += 2;

        return size + size_of_unicode_data;
    }

private:
    size_t str(char * buffer, size_t size) const {
        size_t length = ::snprintf(buffer, size,
            "FileFsVolumeInformation: VolumeCreationTime=%" PRIu64
                " VolumeSerialNumber=0x%X SupportsObjects=%u VolumeLabel=\"%s\"",
            this->VolumeCreationTime, this->VolumeSerialNumber,
            this->SupportsObjects, this->volume_label.c_str());
        return ((length < size) ? length : size - 1);
    }

public:
    inline void log(int level) const {
        char buffer[2048];
        this->str(buffer, sizeof(buffer));
        buffer[sizeof(buffer) - 1] = 0;
        LOG(level, buffer);
    }
};  // FileFsVolumeInformation

}   // namespace fscc

#endif  // #ifndef _REDEMPTION_CORE_FSCC_FILEINFORMATION_HPP_
