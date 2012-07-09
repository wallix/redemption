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

   Module: keywords.hpp
   Description : -

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
