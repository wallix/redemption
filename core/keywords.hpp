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
   Author(s): Christophe Grosjean

   Module: authhook.cpp
   Description : sample post-login authentication data processing

   This code is not actually used in production code, it's just a
   minimal tcp server sample whose purpose is to test the protocol.
   It shows how the post-processing of authentication data works.

   If no such hook is provided redemption is supposed to works seamlessly
   on it's own with consistant behavior.

*/

#if !defined(__KEYWORDS_HPP__)
#define __KEYWORDS_HPP__

enum {
    TYPE_BOOLEAN,
    TYPE_INTEGER,
    TYPE_TEXT
};

struct ProtocolKeyword {
    const char * keyword;
    int type;
    const char * default_value;
};

#endif
