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
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean

*/

#ifndef _REDEMPTION_CHANNELS_RDPDR_RDPDR_HPP_
#define _REDEMPTION_CHANNELS_RDPDR_RDPDR_HPP_


/* NT status codes for RDPDR */
enum {
    STATUS_SUCCESS                 = 0x00000000,
    STATUS_PENDING                 = 0x00000103,

    STATUS_NO_MORE_FILES           = 0x80000006,
    STATUS_DEVICE_PAPER_EMPTY      = 0x8000000e,
    STATUS_DEVICE_POWERED_OFF      = 0x8000000f,
    STATUS_DEVICE_OFF_LINE         = 0x80000010,
    STATUS_DEVICE_BUSY             = 0x80000011,

    STATUS_INVALID_HANDLE          = 0xc0000008,
    STATUS_INVALID_PARAMETER       = 0xc000000d,
    STATUS_NO_SUCH_FILE            = 0xc000000f,
    STATUS_INVALID_DEVICE_REQUEST  = 0xc0000010,
    STATUS_ACCESS_DENIED           = 0xc0000022,
    STATUS_OBJECT_NAME_COLLISION   = 0xc0000035,
    STATUS_DISK_FULL               = 0xc000007f,
    STATUS_FILE_IS_A_DIRECTORY     = 0xc00000ba,
    STATUS_NOT_SUPPORTED           = 0xc00000bb,
    STATUS_TIMEOUT                 = 0xc0000102,
    STATUS_CANCELLED               = 0xc0000120,
};

/* RDPDR constants */
enum {
RDPDR_MAX_DEVICES              = 0x10,
DEVICE_TYPE_SERIAL             = 0x01,
DEVICE_TYPE_PARALLEL           = 0x02,
DEVICE_TYPE_PRINTER            = 0x04,
DEVICE_TYPE_DISK               = 0x08,
DEVICE_TYPE_SCARD              = 0x20,
};

enum {
FILE_DIRECTORY_FILE            = 0x00000001,
FILE_NON_DIRECTORY_FILE        = 0x00000040,
FILE_OPEN_FOR_FREE_SPACE_QUERY = 0x00800000,
};

#endif
