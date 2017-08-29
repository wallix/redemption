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
    Author(s): Clément Moroldo
*/


#pragma once

#include "utils/stream.hpp"

#include <string>

namespace smb2 {

// [MS-SMB2] - 2.2.13 SMB2 CREATE Request
// ======================================

// The SMB2 CREATE Request packet is sent by a client to request either
//  creation of or access to a file. In case of a named pipe or printer, the
//  server MUST create a new file.

// This request is composed of an SMB2 Packet Header, as specified in section
//  2.2.1, that is followed by this request structure.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |         StructureSize         | SecurityFlags |   Requested   |
// |                               |               |  OplockLevel  |
// +-------------------------------+---------------+---------------+
// |                       ImpersonationLevel                      |
// +---------------------------------------------------------------+
// |                         SmbCreateFlags                        |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                            Reserved                           |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+
// |                         DesiredAccess                         |
// +---------------------------------------------------------------+
// |                         FileAttributes                        |
// +---------------------------------------------------------------+
// |                          ShareAccess                          |
// +---------------------------------------------------------------+
// |                       CreateDisposition                       |
// +---------------------------------------------------------------+
// |                         CreateOptions                         |
// +-------------------------------+-------------------------------+
// |           NameOffset          |           NameLength          |
// +-------------------------------+-------------------------------+
// |                      CreateContextsOffset                     |
// +---------------------------------------------------------------+
// |                      CreateContextsLength                     |
// +---------------------------------------------------------------+
// |                       Buffer (variable)                       |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// StructureSize (2 bytes): The client MUST set this field to 57, indicating
//  the size of the request structure, not including the header. The client
//  MUST set it to this value regardless of how long Buffer[] actually is in
//  the request being sent.

// SecurityFlags (1 byte): This field MUST NOT be used and MUST be reserved.
//  The client MUST set this to 0, and the server MUST ignore it.

// RequestedOplockLevel (1 byte): The requested oplock level. This field MUST
//  contain one of the following values.<28> For named pipes, the server MUST
//  always revert to SMB2_OPLOCK_LEVEL_NONE irrespective of the value of this
//  field.

//  +-----------------------------+-------------------------------------------+
//  | Value                       | Meaning                                   |
//  +-----------------------------+-------------------------------------------+
//  | SMB2_OPLOCK_LEVEL_NONE      | No oplock is requested.                   |
//  | 0x00                        |                                           |
//  +-----------------------------+-------------------------------------------+
//  | SMB2_OPLOCK_LEVEL_II        | A level II oplock is requested.           |
//  | 0x01                        |                                           |
//  +-----------------------------+-------------------------------------------+
//  | SMB2_OPLOCK_LEVEL_EXCLUSIVE | An exclusive oplock is requested.         |
//  | 0x08                        |                                           |
//  +-----------------------------+-------------------------------------------+
//  | SMB2_OPLOCK_LEVEL_BATCH     | A batch oplock is requested.              |
//  | 0x09                        |                                           |
//  +-----------------------------+-------------------------------------------+
//  | SMB2_OPLOCK_LEVEL_LEASE     | A lease is requested. If set, the request |
//  | 0xFF                        | packet MUST contain an                    |
//  |                             | SMB2_CREATE_REQUEST_LEASE (section        |
//  |                             | 2.2.13.2.8) create context. This value is |
//  |                             | not valid for the SMB 2.002 dialect.      |
//  +-----------------------------+-------------------------------------------+

// ImpersonationLevel (4 bytes): This field specifies the impersonation level
//  requested by the application that is issuing the create request, and MUST
//  contain one of the following values. The server MUST validate this field,
//  but otherwise ignore it.

//  +----------------+--------------------------------------------------+
//  | Value          | Meaning                                          |
//  +----------------+--------------------------------------------------+
//  | Anonymous      | The application-requested impersonation level is |
//  | 0x00000000     | Anonymous.                                       |
//  +----------------+--------------------------------------------------+
//  | Identification | The application-requested impersonation level is |
//  | 0x00000001     | Identification.                                  |
//  +----------------+--------------------------------------------------+
//  | Impersonation  | The application-requested impersonation level is |
//  | 0x00000002     | Impersonation.                                   |
//  +----------------+--------------------------------------------------+
//  | Delegate       | The application-requested impersonation level is |
//  | 0x00000003     | Delegate.                                        |
//  +----------------+--------------------------------------------------+

//  Impersonation is specified in [MS-WPO] section 9.7; for more information
//  about impersonation, see [MSDN-IMPERS].

// SmbCreateFlags (8 bytes): This field MUST NOT be used and MUST be
//  reserved. The client SHOULD set this field to zero, and the server MUST
//  ignore it on receipt.

// Reserved (8 bytes): This field MUST NOT be used and MUST be reserved. The
//  client sets this to any value, and the server MUST ignore it on receipt.

// DesiredAccess (4 bytes): The level of access that is required, as
//  specified in section 2.2.13.1.

// FileAttributes (4 bytes): This field MUST be a combination of the values
//  specified in [MS-FSCC] section 2.6, and MUST NOT include any values other
//  than those specified in that section.

// ShareAccess (4 bytes): Specifies the sharing mode for the open. If
//  ShareAccess values of FILE_SHARE_READ, FILE_SHARE_WRITE and
//  FILE_SHARE_DELETE are set for a printer file or a named pipe, the server
//  SHOULD<29> ignore these values. The field MUST be constructed using a
//  combination of zero or more of the following bit values.

//  +-------------------+------------------------------------------------------+
//  | Value             | Meaning                                              |
//  +-------------------+------------------------------------------------------+
//  | FILE_SHARE_READ   | When set, indicates that other opens are allowed to  |
//  | 0x00000001        | read this file while this open is present. This bit  |
//  |                   | MUST NOT be set for a named pipe or a printer file.  |
//  |                   | Each open creates a new instance of a named pipe.    |
//  |                   | Likewise, opening a printer file always creates a    |
//  |                   | new file.                                            |
//  +-------------------+------------------------------------------------------+
//  | FILE_SHARE_WRITE  | When set, indicates that other opens are allowed to  |
//  | 0x00000002        | write this file while this open is present. This bit |
//  |                   | MUST NOT be set for a named pipe or a printer file.  |
//  |                   | Each open creates a new instance of a named pipe.    |
//  |                   | Likewise, opening a printer file always creates a    |
//  |                   | new file.                                            |
//  +-------------------+------------------------------------------------------+
//  | FILE_SHARE_DELETE | When set, indicates that other opens are allowed to  |
//  | 0x00000004        | delete or rename this file while this open is        |
//  |                   | present. This bit MUST NOT be set for a named pipe   |
//  |                   | or a printer file. Each open creates a new instance  |
//  |                   | of a named pipe. Likewise, opening a printer file    |
//  |                   | always creates a new file.                           |
//  +-------------------+------------------------------------------------------+

enum : uint32_t {
      FILE_SHARE_READ   = 0x00000001
    , FILE_SHARE_WRITE  = 0x00000002
    , FILE_SHARE_DELETE = 0x00000004
};

static inline
std::string get_ShareAccess_name(uint32_t shareAccess) {

    std::string str;
    (shareAccess & FILE_SHARE_READ) ? str+="FILE_SHARE_READ ":str;
    (shareAccess & FILE_SHARE_WRITE) ? str+="FILE_SHARE_WRITE ":str;
    (shareAccess & FILE_SHARE_DELETE) ? str+="FILE_SHARE_DELETE ":str;

    return str;
}


// CreateDisposition (4 bytes): Defines the action the server MUST take if
//  the file that is specified in the name field already exists. For opening
//  named pipes, this field may be set to any value by the client and MUST be
//  ignored by the server. For other files, this field MUST contain one of
//  the following values.

//  +-------------------+------------------------------------------------------+
//  | Value             | Meaning                                              |
//  +-------------------+------------------------------------------------------+
//  | FILE_SUPERSEDE    | If the file already exists, supersede it. Otherwise, |
//  | 0x00000000        | create the file. This value SHOULD NOT be used for a |
//  |                   | printer object.<30>                                  |
//  +-------------------+------------------------------------------------------+
//  | FILE_OPEN         | If the file already exists, return success;          |
//  | 0x00000001        | otherwise, fail the operation. MUST NOT be used for  |
//  |                   | a printer object.                                    |
//  +-------------------+------------------------------------------------------+
//  | FILE_CREATE       | If the file already exists, fail the operation;      |
//  | 0x00000002        | otherwise, create the file.                          |
//  +-------------------+------------------------------------------------------+
//  | FILE_OPEN_IF      | Open the file if it already exists; otherwise,       |
//  | 0x00000003        | create the file. This value SHOULD NOT be used for a |
//  |                   | printer object.<31>                                  |
//  +-------------------+------------------------------------------------------+
//  | FILE_OVERWRITE    | Overwrite the file if it already exists; otherwise,  |
//  | 0x00000004        | fail the operation. MUST NOT be used for a printer   |
//  |                   | object.                                              |
//  +-------------------+------------------------------------------------------+
//  | FILE_OVERWRITE_IF | Overwrite the file if it already exists; otherwise,  |
//  | 0x00000005        | create the file. This value SHOULD NOT be used for a |
//  |                   | printer object.<32>                                  |
//  +-------------------+------------------------------------------------------+

enum {
      FILE_SUPERSEDE    = 0x00000000
    , FILE_OPEN         = 0x00000001
    , FILE_CREATE       = 0x00000002
    , FILE_OPEN_IF      = 0x00000003
    , FILE_OVERWRITE    = 0x00000004
    , FILE_OVERWRITE_IF = 0x00000005
};

static inline
const char * get_CreateDisposition_name(uint32_t createDisposition) {
    switch (createDisposition) {
        case FILE_SUPERSEDE:    return "FILE_SUPERSEDE";
        case FILE_OPEN:         return "FILE_OPEN";
        case FILE_CREATE:       return "FILE_CREATE";
        case FILE_OPEN_IF:      return "FILE_OPEN_IF";
        case FILE_OVERWRITE:    return "FILE_OVERWRITE";
        case FILE_OVERWRITE_IF: return "FILE_OVERWRITE_IF";
    }

    return "<unknown>";
}

// CreateOptions (4 bytes): Specifies the options to be applied when creating
//  or opening the file. Combinations of the bit positions listed below are
//  valid, unless otherwise noted. This field MUST be constructed using the
//  following values.<33>

//  +--------------------------------+-----------------------------------------+
//  | Value                          | Meaning                                 |
//  +--------------------------------+-----------------------------------------+
//  | FILE_DIRECTORY_FILE            | The file being created or opened is a   |
//  | 0x00000001                     | directory file. With this flag, the     |
//  |                                | CreateDisposition field MUST be set to  |
//  |                                | FILE_CREATE, FILE_OPEN_IF, or           |
//  |                                | FILE_OPEN. With this flag, only the     |
//  |                                | following CreateOptions values are      |
//  |                                | valid: FILE_WRITE_THROUGH,              |
//  |                                | FILE_OPEN_FOR_BACKUP_INTENT,            |
//  |                                | FILE_DELETE_ON_CLOSE, and               |
//  |                                | FILE_OPEN_REPARSE_POINT. If the file    |
//  |                                | being created or opened already exists  |
//  |                                | and is not a directory file and         |
//  |                                | FILE_CREATE is specified in the         |
//  |                                | CreateDisposition field, then the       |
//  |                                | server MUST fail the request with       |
//  |                                | STATUS_OBJECT_NAME_COLLISION. If the    |
//  |                                | file being created or opened already    |
//  |                                | exists and is not a directory file and  |
//  |                                | FILE_CREATE is not specified in the     |
//  |                                | CreateDisposition field, then the       |
//  |                                | server MUST fail the request with       |
//  |                                | STATUS_NOT_A_DIRECTORY. The server MUST |
//  |                                | fail an invalid CreateDisposition field |
//  |                                | or an invalid combination of            |
//  |                                | CreateOptions flags with                |
//  |                                | STATUS_INVALID_PARAMETER.               |
//  +--------------------------------+-----------------------------------------+
//  | FILE_WRITE_THROUGH             | The server MUST propagate writes to     |
//  | 0x00000002                     | this open to persistent storage before  |
//  |                                | returning success to the client on      |
//  |                                | write operations.                       |
//  +--------------------------------+-----------------------------------------+
//  | FILE_SEQUENTIAL_ONLY           | This indicates that the application     |
//  | 0x00000004                     | intends to read or write at sequential  |
//  |                                | offsets using this handle, so the       |
//  |                                | server SHOULD optimize for sequential   |
//  |                                | access. However, the server MUST accept |
//  |                                | any access pattern. This flag value is  |
//  |                                | incompatible with the                   |
//  |                                | FILE_RANDOM_ACCESS value.               |
//  +--------------------------------+-----------------------------------------+
//  | FILE_NO_INTERMEDIATE_BUFFERING | The server or underlying object store   |
//  | 0x00000008                     | SHOULD NOT cache data at intermediate   |
//  |                                | layers and SHOULD allow it to flow      |
//  |                                | through to persistent storage.          |
//  +--------------------------------+-----------------------------------------+
//  | FILE_SYNCHRONOUS_IO_ALERT      | This bit SHOULD be set to 0 and MUST be |
//  | 0x00000010                     | ignored by the server.<34>              |
//  +--------------------------------+-----------------------------------------+
//  | FILE_SYNCHRONOUS_IO_NONALERT   | This bit SHOULD be set to 0 and MUST be |
//  | 0x00000020                     | ignored by the server.<35>              |
//  +--------------------------------+-----------------------------------------+
//  | FILE_NON_DIRECTORY_FILE        | If the name of the file being created   |
//  | 0x00000040                     | or opened matches with an existing      |
//  |                                | directory file, the server MUST fail    |
//  |                                | the request with                        |
//  |                                | STATUS_FILE_IS_A_DIRECTORY. This flag   |
//  |                                | MUST NOT be used with                   |
//  |                                | FILE_DIRECTORY_FILE or the server MUST  |
//  |                                | fail the request with                   |
//  |                                | STATUS_INVALID_PARAMETER.               |
//  +--------------------------------+-----------------------------------------+
//  | FILE_COMPLETE_IF_OPLOCKED      | This bit SHOULD be set to 0 and MUST be |
//  | 0x00000100                     | ignored by the server.<36>              |
//  +--------------------------------+-----------------------------------------+
//  | FILE_NO_EA_KNOWLEDGE           | The caller does not understand how to   |
//  | 0x00000200                     | handle extended attributes. If the      |
//  |                                | request includes an                     |
//  |                                | SMB2_CREATE_EA_BUFFER create context,   |
//  |                                | then the server MUST fail this request  |
//  |                                | with STATUS_ACCESS_DENIED. If extended  |
//  |                                | attributes with the FILE_NEED_EA flag   |
//  |                                | (see [MS-FSCC] section 2.4.15) set are  |
//  |                                | associated with the file being opened,  |
//  |                                | then the server MUST fail this request  |
//  |                                | with STATUS_ACCESS_DENIED.              |
//  +--------------------------------+-----------------------------------------+
//  | FILE_RANDOM_ACCESS             | This indicates that the application     |
//  | 0x00000800                     | intends to read or write at random      |
//  |                                | offsets using this handle, so the       |
//  |                                | server SHOULD optimize for random       |
//  |                                | access. However, the server MUST accept |
//  |                                | any access pattern. This flag value is  |
//  |                                | incompatible with the                   |
//  |                                | FILE_SEQUENTIAL_ONLY value. If both     |
//  |                                | FILE_RANDOM_ACCESS and                  |
//  |                                | FILE_SEQUENTIAL_ONLY are set, then      |
//  |                                | FILE_SEQUENTIAL_ONLY is ignored.        |
//  +--------------------------------+-----------------------------------------+
//  | FILE_DELETE_ON_CLOSE           | The file MUST be automatically deleted  |
//  | 0x00001000                     | when the last open request on this file |
//  |                                | is closed. When this option is set, the |
//  |                                | DesiredAccess field MUST include the    |
//  |                                | DELETE flag. This option is often used  |
//  |                                | for temporary files.                    |
//  +--------------------------------+-----------------------------------------+
//  | FILE_OPEN_BY_FILE_ID           | This bit SHOULD be set to 0 and the     |
//  | 0x00002000                     | server MUST fail the request with a     |
//  |                                | STATUS_NOT_SUPPORTED error if this bit  |
//  |                                | is set.<37>                             |
//  +--------------------------------+-----------------------------------------+
//  | FILE_OPEN_FOR_BACKUP_INTENT    | The file is being opened for backup     |
//  | 0x00004000                     | intent. That is, it is being opened or  |
//  |                                | created for the purposes of either a    |
//  |                                | backup or a restore operation. The      |
//  |                                | server can check to ensure that the     |
//  |                                | caller is capable of overriding         |
//  |                                | whatever security checks have been      |
//  |                                | placed on the file to allow a backup or |
//  |                                | restore operation to occur. The server  |
//  |                                | can check for access rights to the file |
//  |                                | before checking the DesiredAccess       |
//  |                                | field.                                  |
//  +--------------------------------+-----------------------------------------+
//  | FILE_NO_COMPRESSION            | The file cannot be compressed. This bit |
//  | 0x00008000                     | is ignored when FILE_DIRECTORY_FILE is  |
//  |                                | set in CreateOptions.                   |
//  +--------------------------------+-----------------------------------------+
//  | FILE_OPEN_REMOTE_INSTANCE      | This bit SHOULD be set to 0 and MUST be |
//  | 0x00000400                     | ignored by the server.                  |
//  +--------------------------------+-----------------------------------------+
//  | FILE_OPEN_REQUIRING_OPLOCK     | This bit SHOULD be set to 0 and MUST be |
//  | 0x00010000                     | ignored by the server.                  |
//  +--------------------------------+-----------------------------------------+
//  | FILE_DISALLOW_EXCLUSIVE        | This bit SHOULD be set to 0 and MUST be |
//  | 0x00020000                     | ignored by the server.                  |
//  +--------------------------------+-----------------------------------------+
//  | FILE_RESERVE_OPFILTER          | This bit SHOULD be set to 0 and the     |
//  | 0x00100000                     | server MUST fail the request with a     |
//  |                                | STATUS_NOT_SUPPORTED error if this bit  |
//  |                                | is set.<38>                             |
//  +--------------------------------+-----------------------------------------+
//  | FILE_OPEN_REPARSE_POINT        | If the file or directory being opened   |
//  | 0x00200000                     | is a reparse point, open the reparse    |
//  |                                | point itself rather than the target     |
//  |                                | that the reparse point references.      |
//  +--------------------------------+-----------------------------------------+
//  | FILE_OPEN_NO_RECALL            | In an HSM (Hierarchical Storage         |
//  | 0x00400000                     | Management) environment, this flag      |
//  |                                | means the file SHOULD NOT be recalled   |
//  |                                | from tertiary storage such as tape. The |
//  |                                | recall can take several minutes. The    |
//  |                                | caller can specify this flag to avoid   |
//  |                                | those delays.                           |
//  +--------------------------------+-----------------------------------------+
//  | FILE_OPEN_FOR_FREE_SPACE_QUERY | Open file to query for free space. The  |
//  | 0x00800000                     | client SHOULD set this to 0 and the     |
//  |                                | server MUST ignore it.<39>              |
//  +--------------------------------+-----------------------------------------+

enum {
      FILE_DIRECTORY_FILE            = 0x00000001
    , FILE_WRITE_THROUGH             = 0x00000002
    , FILE_SEQUENTIAL_ONLY           = 0x00000004
    , FILE_NO_INTERMEDIATE_BUFFERING = 0x00000008
    , FILE_SYNCHRONOUS_IO_ALERT      = 0x00000010
    , FILE_SYNCHRONOUS_IO_NONALERT   = 0x00000020
    , FILE_NON_DIRECTORY_FILE        = 0x00000040
    , FILE_COMPLETE_IF_OPLOCKED      = 0x00000100
    , FILE_NO_EA_KNOWLEDGE           = 0x00000200
    , FILE_RANDOM_ACCESS             = 0x00000800
    , FILE_DELETE_ON_CLOSE           = 0x00001000
    , FILE_OPEN_BY_FILE_ID           = 0x00002000
    , FILE_OPEN_FOR_BACKUP_INTENT    = 0x00004000
    , FILE_NO_COMPRESSION            = 0x00008000
    , FILE_OPEN_REMOTE_INSTANCE      = 0x00000400
    , FILE_OPEN_REQUIRING_OPLOCK     = 0x00010000
    , FILE_DISALLOW_EXCLUSIVE        = 0x00020000
    , FILE_RESERVE_OPFILTER          = 0x00100000
    , FILE_OPEN_REPARSE_POINT        = 0x00200000
    , FILE_OPEN_NO_RECALL            = 0x00400000
    , FILE_OPEN_FOR_FREE_SPACE_QUERY = 0x00800000
};

static inline
std::string get_CreateOptions_name(uint32_t createOptions) {

    std::string str;

    (createOptions & FILE_DIRECTORY_FILE) ? str+="FILE_DIRECTORY_FILE " :str;
    (createOptions & FILE_WRITE_THROUGH) ? str+="FILE_WRITE_THROUGH " :str;
    (createOptions & FILE_SEQUENTIAL_ONLY) ? str+="FILE_SEQUENTIAL_ONLY " :str;
    (createOptions & FILE_NO_INTERMEDIATE_BUFFERING) ? str+="FILE_NO_INTERMEDIATE_BUFFERING " : str;
    (createOptions & FILE_SYNCHRONOUS_IO_ALERT) ? str+="FILE_SYNCHRONOUS_IO_ALERT " : str;
    (createOptions & FILE_SYNCHRONOUS_IO_NONALERT) ? str+="FILE_SYNCHRONOUS_IO_NONALERT " : str;
    (createOptions & FILE_NON_DIRECTORY_FILE) ? str+="FILE_NON_DIRECTORY_FILE " :str;
    (createOptions & FILE_COMPLETE_IF_OPLOCKED) ? str+="FILE_COMPLETE_IF_OPLOCKED " : str;
    (createOptions & FILE_NO_EA_KNOWLEDGE) ? str+="FILE_NO_EA_KNOWLEDGE " : str;
    (createOptions & FILE_RANDOM_ACCESS) ? str+="FILE_RANDOM_ACCESS " : str;
    (createOptions & FILE_DELETE_ON_CLOSE) ? str+="FILE_DELETE_ON_CLOSE ":str;
    (createOptions & FILE_OPEN_BY_FILE_ID) ? str+="FILE_OPEN_BY_FILE_ID " : str;
    (createOptions & FILE_OPEN_FOR_BACKUP_INTENT) ? str+="FILE_OPEN_FOR_BACKUP_INTENT " : str;
    (createOptions & FILE_NO_COMPRESSION) ? str+="FILE_NO_COMPRESSION " : str;
    (createOptions & FILE_OPEN_REMOTE_INSTANCE) ? str+="FILE_OPEN_REMOTE_INSTANCE " : str;
    (createOptions & FILE_OPEN_REQUIRING_OPLOCK) ? str+="FILE_OPEN_REQUIRING_OPLOCK " : str;
    (createOptions & FILE_DISALLOW_EXCLUSIVE) ? str+="FILE_DISALLOW_EXCLUSIVE " : str;
    (createOptions & FILE_RESERVE_OPFILTER) ? str+="FILE_RESERVE_OPFILTER " : str;
    (createOptions & FILE_OPEN_REPARSE_POINT) ? str+="FILE_OPEN_REPARSE_POINT " : str;
    (createOptions & FILE_OPEN_NO_RECALL) ? str+="FILE_OPEN_NO_RECALL " : str;
    (createOptions & FILE_OPEN_FOR_FREE_SPACE_QUERY) ? str+="FILE_OPEN_FOR_FREE_SPACE_QUERY " : str;

    return str;
}



// NameOffset (2 bytes): The offset, in bytes, from the beginning of the SMB2
//  header to the 8-byte aligned file name. If SMB2_FLAGS_DFS_OPERATIONS is
//  set in the Flags field of the SMB2 header, the file name can be prefixed
//  with DFS link information that will be removed during DFS name
//  normalization as specified in section 3.3.5.9. Otherwise, the file name
//  is relative to the share that is identified by the TreeId in the SMB2
//  header. The NameOffset field SHOULD be set to the offset of the Buffer
//  field from the beginning of the SMB2 header. The file name (after DFS
//  normalization if needed) MUST conform to the specification of a relative
//  pathname in [MS-FSCC] section 2.1.5. A zero length file name indicates a
//  request to open the root of the share.

// NameLength (2 bytes): The length of the file name, in bytes. If no file
//  name is provided, this field MUST be set to 0.

// CreateContextsOffset (4 bytes): The offset, in bytes, from the beginning
//  of the SMB2 header to the first 8-byte aligned SMB2_CREATE_CONTEXT
//  structure in the request. If no SMB2_CREATE_CONTEXTs are being sent, this
//  value MUST be 0.

// CreateContextsLength (4 bytes): The length, in bytes, of the list of
//  SMB2_CREATE_CONTEXT structures sent in this request.

// Buffer (variable): A variable-length buffer that contains the Unicode file
//  name and create context list, as defined by NameOffset, NameLength,
//  CreateContextsOffset, and CreateContextsLength. In the request, the
//  Buffer field MUST be at least one byte in length. The file name (after
//  DFS normalization if needed) MUST conform to the specification of a
//  relative pathname in [MS-FSCC] section 2.1.5.

// [MS-SMB2] - 2.2.13.1 SMB2 Access Mask Encoding
// ==============================================

// The SMB2 Access Mask Encoding in SMB2 is a 4-byte bit field value that
//  contains an array of flags. An access mask can specify access for one of
//  two basic groups: either for a file, pipe, or printer (specified in
//  section 2.2.13.1.1) or for a directory (specified in section 2.2.13.1.2).
//  Each access mask MUST be a combination of zero or more of the bit
//  positions that are shown below.

// [MS-SMB2] - 2.2.13.1.1 File_Pipe_Printer_Access_Mask
// ====================================================

// The following SMB2 Access Mask flag values can be used when accessing a
//  file, pipe or printer.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                 File_Pipe_Printer_Access_Mask                 |
// +---------------------------------------------------------------+

// File_Pipe_Printer_Access_Mask (4 bytes): For a file, pipe, or printer, the
//  value MUST be constructed using the following values (for a printer, the
//  value MUST have at least one of the following: FILE_WRITE_DATA,
//  FILE_APPEND_DATA, or GENERIC_WRITE).

//  +------------------------+-------------------------------------------------+
//  | Value                  | Meaning                                         |
//  +------------------------+-------------------------------------------------+
//  | FILE_READ_DATA         | This value indicates the right to read data     |
//  | 0x00000001             | from the file or named pipe.                    |
//  +------------------------+-------------------------------------------------+
//  | FILE_WRITE_DATA        | This value indicates the right to write data    |
//  | 0x00000002             | into the file or named pipe beyond the end of   |
//  |                        | the file.                                       |
//  +------------------------+-------------------------------------------------+
//  | FILE_APPEND_DATA       | This value indicates the right to append data   |
//  | 0x00000004             | into the file or named pipe.                    |
//  +------------------------+-------------------------------------------------+
//  | FILE_READ_EA           | This value indicates the right to read the      |
//  | 0x00000008             | extended attributes of the file or named pipe.  |
//  +------------------------+-------------------------------------------------+
//  | FILE_WRITE_EA          | This value indicates the right to write or      |
//  | 0x00000010             | change the extended attributes to the file or   |
//  |                        | named pipe.                                     |
//  +------------------------+-------------------------------------------------+
//  | FILE_DELETE_CHILD      | This value indicates the right to delete        |
//  | 0x00000040             | entries within a directory.                     |
//  +------------------------+-------------------------------------------------+
//  | FILE_EXECUTE           | This value indicates the right to execute the   |
//  | 0x00000020             | file.                                           |
//  +------------------------+-------------------------------------------------+
//  | FILE_READ_ATTRIBUTES   | This value indicates the right to read the      |
//  | 0x00000080             | attributes of the file.                         |
//  +------------------------+-------------------------------------------------+
//  | FILE_WRITE_ATTRIBUTES  | This value indicates the right to change the    |
//  | 0x00000100             | attributes of the file.                         |
//  +------------------------+-------------------------------------------------+
//  | DELETE                 | This value indicates the right to delete the    |
//  | 0x00010000             | file.                                           |
//  +------------------------+-------------------------------------------------+
//  | READ_CONTROL           | This value indicates the right to read the      |
//  | 0x00020000             | security descriptor for the file or named pipe. |
//  +------------------------+-------------------------------------------------+
//  | WRITE_DAC              | This value indicates the right to change the    |
//  | 0x00040000             | discretionary access control list (DACL) in the |
//  |                        | security descriptor for the file or named pipe. |
//  |                        | For the DACL data structure, see ACL in         |
//  |                        | [MS-DTYP].                                      |
//  +------------------------+-------------------------------------------------+
//  | WRITE_OWNER            | This value indicates the right to change the    |
//  | 0x00080000             | owner in the security descriptor for the file   |
//  |                        | or named pipe.                                  |
//  +------------------------+-------------------------------------------------+
//  | SYNCHRONIZE            | SMB2 clients set this flag to any value.<40>    |
//  | 0x00100000             | SMB2 servers SHOULD<41> ignore this flag.       |
//  +------------------------+-------------------------------------------------+
//  | ACCESS_SYSTEM_SECURITY | This value indicates the right to read or       |
//  | 0x01000000             | change the system access control list (SACL) in |
//  |                        | the security descriptor for the file or named   |
//  |                        | pipe. For the SACL data structure, see ACL in   |
//  |                        | [MS-DTYP].<42>                                  |
//  +------------------------+-------------------------------------------------+
//  | MAXIMUM_ALLOWED        | This value indicates that the client is         |
//  | 0x02000000             | requesting an open to the file with the highest |
//  |                        | level of access the client has on this file. If |
//  |                        | no access is granted for the client on this     |
//  |                        | file, the server MUST fail the open with        |
//  |                        | STATUS_ACCESS_DENIED.                           |
//  +------------------------+-------------------------------------------------+
//  | GENERIC_ALL            | This value indicates a request for all the      |
//  | 0x10000000             | access flags that are previously listed except  |
//  |                        | MAXIMUM_ALLOWED and ACCESS_SYSTEM_SECURITY.     |
//  +------------------------+-------------------------------------------------+
//  | GENERIC_EXECUTE        | This value indicates a request for the          |
//  | 0x20000000             | following combination of access flags listed    |
//  |                        | above: FILE_READ_ATTRIBUTES| FILE_EXECUTE|      |
//  |                        | SYNCHRONIZE| READ_CONTROL.                      |
//  +------------------------+-------------------------------------------------+
//  | GENERIC_WRITE          | This value indicates a request for the          |
//  | 0x40000000             | following combination of access flags listed    |
//  |                        | above: FILE_WRITE_DATA| FILE_APPEND_DATA|       |
//  |                        | FILE_WRITE_ATTRIBUTES| FILE_WRITE_EA|           |
//  |                        | SYNCHRONIZE| READ_CONTROL.                      |
//  +------------------------+-------------------------------------------------+
//  | GENERIC_READ           | This value indicates a request for the          |
//  | 0x80000000             | following combination of access flags listed    |
//  |                        | above: FILE_READ_DATA| FILE_READ_ATTRIBUTES|    |
//  |                        | FILE_READ_EA| SYNCHRONIZE| READ_CONTROL.        |
//  +------------------------+-------------------------------------------------+

enum {
      FILE_READ_DATA         = 0x00000001
    , FILE_WRITE_DATA        = 0x00000002
    , FILE_APPEND_DATA       = 0x00000004
    , FILE_READ_EA           = 0x00000008
    , FILE_WRITE_EA          = 0x00000010
    , FILE_DELETE_CHILD      = 0x00000040
    , FILE_EXECUTE           = 0x00000020
    , FILE_READ_ATTRIBUTES   = 0x00000080
    , FILE_WRITE_ATTRIBUTES  = 0x00000100
    , DELETE                 = 0x00010000
    , READ_CONTROL           = 0x00020000
    , WRITE_DAC              = 0x00040000
    , WRITE_OWNER            = 0x00080000
    , SYNCHRONIZE            = 0x00100000
    , ACCESS_SYSTEM_SECURITY = 0x01000000
    , MAXIMUM_ALLOWED        = 0x02000000
    , GENERIC_ALL            = 0x10000000
    , GENERIC_EXECUTE        = 0x20000000
    , GENERIC_WRITE          = 0x40000000
    , GENERIC_READ           = 0x80000000
};

static inline
std::string get_File_Pipe_Printer_Access_Mask_name(uint32_t File_Pipe_Printer_Access_Mask) {

    std::string str;
    (File_Pipe_Printer_Access_Mask & FILE_READ_DATA) ? str+="FILE_READ_DATA " :str;
    (File_Pipe_Printer_Access_Mask & FILE_WRITE_DATA) ? str+="FILE_WRITE_DATA " :str;
    (File_Pipe_Printer_Access_Mask & FILE_APPEND_DATA) ? str+="FILE_APPEND_DATA " :str;
    (File_Pipe_Printer_Access_Mask & FILE_READ_EA) ? str+="FILE_READ_EA " : str;
    (File_Pipe_Printer_Access_Mask & FILE_WRITE_EA) ? str+="FILE_WRITE_EA " : str;
    (File_Pipe_Printer_Access_Mask & FILE_DELETE_CHILD) ? str+="FILE_DELETE_CHILD " : str;
    (File_Pipe_Printer_Access_Mask & FILE_EXECUTE) ? str+="FILE_EXECUTE " :str;
    (File_Pipe_Printer_Access_Mask & FILE_READ_ATTRIBUTES) ? str+="FILE_READ_ATTRIBUTES " : str;
    (File_Pipe_Printer_Access_Mask & FILE_WRITE_ATTRIBUTES) ? str+="FILE_WRITE_ATTRIBUTES " : str;
    (File_Pipe_Printer_Access_Mask & DELETE) ? str+="DELETE " : str;
    (File_Pipe_Printer_Access_Mask & READ_CONTROL) ? str+="READ_CONTROL ":str;
    (File_Pipe_Printer_Access_Mask & WRITE_DAC) ? str+="WRITE_DAC " : str;
    (File_Pipe_Printer_Access_Mask & WRITE_OWNER) ? str+="WRITE_OWNER " : str;
    (File_Pipe_Printer_Access_Mask & SYNCHRONIZE) ? str+="SYNCHRONIZE " : str;
    (File_Pipe_Printer_Access_Mask & ACCESS_SYSTEM_SECURITY) ? str+="ACCESS_SYSTEM_SECURITY " : str;
    (File_Pipe_Printer_Access_Mask & MAXIMUM_ALLOWED) ? str+="MAXIMUM_ALLOWED " : str;
    (File_Pipe_Printer_Access_Mask & GENERIC_ALL) ? str+="GENERIC_ALL " : str;
    (File_Pipe_Printer_Access_Mask & GENERIC_EXECUTE) ? str+="GENERIC_EXECUTE " : str;
    (File_Pipe_Printer_Access_Mask & GENERIC_WRITE) ? str+="GENERIC_WRITE " : str;
    (File_Pipe_Printer_Access_Mask & GENERIC_READ) ? str+="GENERIC_READ " : str;

    return str;
}

static inline
bool read_access_is_required(uint32_t DesiredAccess, bool strict_check) {
    uint32_t values_of_strict_checking = (FILE_READ_EA |
                                          FILE_READ_ATTRIBUTES |
                                          READ_CONTROL |
                                          ACCESS_SYSTEM_SECURITY |
                                          MAXIMUM_ALLOWED
                                         );

    return (DesiredAccess &
            (FILE_READ_DATA |
             FILE_EXECUTE |
             GENERIC_ALL |
             GENERIC_EXECUTE |
             GENERIC_READ |
             (strict_check ? values_of_strict_checking : 0)
            )
           );
}

static inline
bool write_access_is_required(uint32_t DesiredAccess, bool strict_check) {
    uint32_t values_of_strict_checking = (FILE_WRITE_EA |
                                          FILE_WRITE_ATTRIBUTES |
                                          WRITE_DAC |
                                          WRITE_OWNER |
                                          ACCESS_SYSTEM_SECURITY |
                                          MAXIMUM_ALLOWED
                                         );

    return (DesiredAccess &
            (FILE_WRITE_DATA |
             FILE_APPEND_DATA |
             FILE_DELETE_CHILD |
             DELETE |
             GENERIC_ALL |
             GENERIC_WRITE |
             (strict_check ? values_of_strict_checking : 0)
            )
           );
}



//2.2.13.1.2 Directory_Access_Mask

// The following SMB2 Access Mask flag values can be used when accessing a directory.

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                    Directory_Access_Mask                      |
// +---------------------------------------------------------------+

// Directory_Access_Mask (4 bytes): For a directory, the value MUST be constructed using the following values:

//  +------------------------+-------------------------------------------------+
//  | Value                  | Meaning                                         |
//  +------------------------+-------------------------------------------------+
//  | FILE_LIST_DIRECTORY    | This value indicates the right to enumerate the |
//  | 0x00000001             | contents of the directory.                      |
//  +------------------------+-------------------------------------------------+
//  | FILE_ADD_FILE          | This value indicates the right to create a file |
//  | 0x00000002             | under the directory.                            |
//  +------------------------+-------------------------------------------------+
//  | FILE_ADD_SUBDIRECTORY  | This value indicates the right to add a         |
//  | 0x00000004             | sub-directory under the directory.              |
//  +------------------------+-------------------------------------------------+
//  | FILE_READ_EA           | This value indicates the right to read the      |
//  | 0x00000008             | extended attributes of the directory.           |
//  +------------------------+-------------------------------------------------+
//  | FILE_WRITE_EA          | This value indicates the right to write or      |
//  | 0x00000010             | change the extended attributes of the directory.|
//  +------------------------+-------------------------------------------------+
//  | FILE_TRAVERSE          | This value indicates the right to traverse this |
//  | 0x00000020             | directory if the server enforces traversal      |
//  |                        | checking.                                       |
//  +------------------------+-------------------------------------------------+
//  | FILE_DELETE_CHILD      | This value indicates the right to delete the    |
//  | 0x00000040             | files and directories within this directory.    |
//  +------------------------+-------------------------------------------------+
//  | FILE_READ_ATTRIBUTES   | This value indicates the right to read the      |
//  | 0x00000080             | attributes of the directory.                    |
//  +------------------------+-------------------------------------------------+
//  | FILE_WRITE_ATTRIBUTES  | This value indicates the right to change the    |
//  | 0x00000100             | attributes of the directory.                    |
//  +------------------------+-------------------------------------------------+
//  | DELETE                 | This value indicates the right to delete the    |
//  | 0x00010000             | directory.                                      |
//  +------------------------+-------------------------------------------------+
//  | READ_CONTROL           | This value indicates the right to read the      |
//  | 0x00020000             | security descriptor for the directory.          |
//  |                        |                                                 |
//  +------------------------+-------------------------------------------------+
//  | WRITE_DAC              | This value indicates the right to change the    |
//  | 0x00040000             | DACL in the security descriptor for the         |
//  |                        | directory. For the DACL data structure, see ACL |
//  |                        | in [MS-DTYP].                                   |
//  +------------------------+-------------------------------------------------+
//  | WRITE_OWNER            | This value indicates the right to change the    |
//  | 0x00080000             | owner in the security descriptor for the        |
//  |                        | directory.                                      |
//  +------------------------+-------------------------------------------------+
//  | SYNCHRONIZE            | SMB2 clients set this flag to any value.<45>    |
//  | 0x00100000             | SMB2 servers SHOULD<46> ignore this flag.       |
//  +------------------------+-------------------------------------------------+
//  | ACCESS_SYSTEM_SECURITY | This value indicates the right to read or change|
//  | 0x01000000             | the SACL in the security descriptor for the     |
//  |                        | directory. For the SACL data structure, see ACL |
//  |                        | in [MS-DTYP].<47>                               |
//  +------------------------+-------------------------------------------------+
//  | MAXIMUM_ALLOWED        | This value indicates that the client is         |
//  | 0x02000000             | requesting an open to the directory with the    |
//  |                        | highest level of access the client has on this  |
//  |                        | directory. If no access is granted for the      |
//  |                        | client on this directory, the server MUST fail  |
//  |                        | the open with STATUS_ACCESS_DENIED.             |
//  +------------------------+-------------------------------------------------+
//  | GENERIC_ALL            | This value indicates a request for all the      |
//  | 0x10000000             | access flags that are listed above except       |
//  |                        | MAXIMUM_ALLOWED and ACCESS_SYSTEM_SECURITY.     |
//  +------------------------+-------------------------------------------------+
//  | GENERIC_EXECUTE        | TThis value indicates a request for the         |
//  | 0x20000000             | following access flags listed above:            |
//  |                        | FILE_READ_ATTRIBUTES| FILE_TRAVERSE|            |
//  |                        | SYNCHRONIZE| READ_CONTROL.                      |
//  +------------------------+-------------------------------------------------+
//  | GENERIC_WRITE          | This value indicates a request for the following|
//  | 0x40000000             | access flags listed above: FILE_ADD_FILE|       |
//  |                        | FILE_ADD_SUBDIRECTORY| FILE_WRITE_ATTRIBUTES|   |
//  |                        | FILE_WRITE_EA| SYNCHRONIZE| READ_CONTROL.       |
//  +------------------------+-------------------------------------------------+
//  | GENERIC_READ           | This value indicates a request for the following|
//  | 0x80000000             | access flags listed above: FILE_LIST_DIRECTORY| |
//  |                        | FILE_READ_ATTRIBUTES| FILE_READ_EA|             |
//  |                        | SYNCHRONIZE| READ_CONTROL.                      |
//  +------------------------+-------------------------------------------------+

enum : uint32_t {
    FILE_LIST_DIRECTORY    = 0x00000001,
    FILE_ADD_FILE          = 0x00000002,
    FILE_ADD_SUBDIRECTORY  = 0x00000004,
    //FILE_READ_EA           = 0x00000008,
    //FILE_WRITE_EA          = 0x00000010,
    FILE_TRAVERSE          = 0x00000020,
    //FILE_DELETE_CHILD      = 0x00000040,
    //FILE_READ_ATTRIBUTES   = 0x00000080,
    //FILE_WRITE_ATTRIBUTES  = 0x00000100,
    //DELETE                 = 0x00010000,
    //READ_CONTROL           = 0x00020000,
    //WRITE_DAC              = 0x00040000,
    //WRITE_OWNER            = 0x00080000,
    //SYNCHRONIZE            = 0x00100000,
    //ACCESS_SYSTEM_SECURITY = 0x01000000,
    //MAXIMUM_ALLOWED        = 0x02000000,
    //GENERIC_ALL            = 0x10000000,
    //GENERIC_EXECUTE        = 0x20000000,
    //GENERIC_WRITE          = 0x40000000,
    //GENERIC_READ           = 0x80000000
};

static inline
std::string get_Directory_Access_Mask_name(uint32_t Directory_Access_Mask) {

    std::string str;
    (Directory_Access_Mask & FILE_LIST_DIRECTORY) ? str+="FILE_LIST_DIRECTORY " :str;
    (Directory_Access_Mask & FILE_ADD_FILE) ? str+="FILE_ADD_FILE " :str;
    (Directory_Access_Mask & FILE_ADD_SUBDIRECTORY) ? str+="FILE_ADD_SUBDIRECTORY " :str;
    (Directory_Access_Mask & FILE_READ_EA) ? str+="FILE_READ_EA " : str;
    (Directory_Access_Mask & FILE_WRITE_EA) ? str+="FILE_WRITE_EA " : str;
    (Directory_Access_Mask & FILE_TRAVERSE) ? str+="FILE_TRAVERSE " : str;
    (Directory_Access_Mask & FILE_DELETE_CHILD) ? str+="FILE_DELETE_CHILD " :str;
    (Directory_Access_Mask & FILE_READ_ATTRIBUTES) ? str+="FILE_READ_ATTRIBUTES " : str;
    (Directory_Access_Mask & FILE_WRITE_ATTRIBUTES) ? str+="FILE_WRITE_ATTRIBUTES " : str;
    (Directory_Access_Mask & DELETE) ? str+="DELETE " : str;
    (Directory_Access_Mask & READ_CONTROL) ? str+="READ_CONTROL ":str;
    (Directory_Access_Mask & WRITE_DAC) ? str+="WRITE_DAC " : str;
    (Directory_Access_Mask & WRITE_OWNER) ? str+="WRITE_OWNER " : str;
    (Directory_Access_Mask & SYNCHRONIZE) ? str+="SYNCHRONIZE " : str;
    (Directory_Access_Mask & ACCESS_SYSTEM_SECURITY) ? str+="ACCESS_SYSTEM_SECURITY " : str;
    (Directory_Access_Mask & MAXIMUM_ALLOWED) ? str+="MAXIMUM_ALLOWED " : str;
    (Directory_Access_Mask & GENERIC_ALL) ? str+="GENERIC_ALL " : str;
    (Directory_Access_Mask & GENERIC_EXECUTE) ? str+="GENERIC_EXECUTE " : str;
    (Directory_Access_Mask & GENERIC_WRITE) ? str+="GENERIC_WRITE " : str;
    (Directory_Access_Mask & GENERIC_READ) ? str+="GENERIC_READ " : str;

    return str;
}



// 2.2.36 SMB2 CHANGE_NOTIFY Response

// The SMB2 CHANGE_NOTIFY Response packet is sent by the server to transmit the results of a client's SMB2 CHANGE_NOTIFY Request (section 2.2.35). The server MUST send this packet only if a change occurs and MUST NOT send this packet otherwise. An SMB2 CHANGE_NOTIFY Request (section 2.2.35) will result in, at most, one response from the server. A server can choose to aggregate multiple changes into the same change notify response. The server MUST include at least one FILE_NOTIFY_INFORMATION structure if it detects a change. This response consists of an SMB2 header, as specified in section 2.2.1, followed by this response structure:

// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// | | | | | | | | | | |1| | | | | | | | | |2| | | | | | | | | |3| |
// |0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|2|3|4|5|6|7|8|9|0|1|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |         StructureSize         |      OutputBufferOffset       |
// +---------------------------------------------------------------+
// |                       OutputBufferLength                      |
// +---------------------------------------------------------------+
// |                        Buffer (variable)                      |
// +---------------------------------------------------------------+
// |                              ...                              |
// +---------------------------------------------------------------+

// StructureSize (2 bytes): The server MUST set this field to 9, indicating the size of the request structure, not including the header. The server MUST set the field to this value regardless of how long Buffer[] actually is in the request being sent.

// OutputBufferOffset (2 bytes): The offset, in bytes, from the beginning of the SMB2 header to the change information being returned.

// OutputBufferLength (4 bytes): The length, in bytes, of the change information being returned.

// Buffer (variable): A variable-length buffer containing the change information being returned in the response, as described by the OutputBufferOffset and OutputBufferLength fields. This field is an array of FILE_NOTIFY_INFORMATION structures, as specified in [MS-FSCC] section 2.4.42.

struct ChangeNotifyResponse {

    uint16_t StructureSize = 0;
    uint16_t OutputBufferOffset = 0;
    uint32_t OutputBufferLength = 0;

    ChangeNotifyResponse() = default;

    ChangeNotifyResponse( uint16_t StructureSize, uint16_t OutputBufferOffset,
    uint32_t OutputBufferLength)
      : StructureSize(StructureSize)
      , OutputBufferOffset(OutputBufferOffset)
      , OutputBufferLength(OutputBufferLength)
      {}

    void emit(OutStream & stream) const {
        stream.out_uint16_le(this->StructureSize);
        stream.out_uint16_le(this->OutputBufferOffset);
        stream.out_uint32_le(this->OutputBufferLength);
    }

    void receive(InStream & stream) {
        this->StructureSize = stream.in_uint16_le();
        this->OutputBufferOffset = stream.in_uint16_le();
        this->OutputBufferLength = stream.in_uint32_le();
    }

    void log() const {
        LOG(LOG_INFO, "     File Disposition Information:");
        LOG(LOG_INFO, "          * StructureSize      = %d (2 bytes)", int(this->StructureSize));
        LOG(LOG_INFO, "          * OutputBufferOffset = %d (2 bytes)", int(this->OutputBufferOffset));
        LOG(LOG_INFO, "          * OutputBufferLength = %d (4 bytes)", int(this->OutputBufferLength));
    }
};


}   // namespace smb2

