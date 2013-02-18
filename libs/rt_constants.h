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

   Constants used by RT lib

*/

#ifndef _REDEMPTION_LIBS_RT_CONSTANTS_H_
#define _REDEMPTION_LIBS_RT_CONSTANTS_H_

typedef enum {
    RT_ERROR_OK,
    RT_ERROR_MALLOC,
    RT_ERROR_EOF,
    RT_ERROR_RECV_ONLY,
    RT_ERROR_SEND_ONLY,
    RT_ERROR_DATA_MISMATCH,
    RT_ERROR_TYPE_MISMATCH,
    RT_ERROR_UNKNOWN_TYPE,
    RT_ERROR_DIFFERS,
    RT_ERROR_NOT_IMPLEMENTED,
} RT_ERROR;


#endif
