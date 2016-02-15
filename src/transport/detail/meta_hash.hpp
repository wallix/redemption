/*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*   Product name: redemption, a FLOSS RDP proxy
*   Copyright (C) Wallix 2010-2015
*   Author(s): Jonathan Poelen
*/

#ifndef REDEMPTION_TRANSPORT_DETAIL_META_HASH_HPP
#define REDEMPTION_TRANSPORT_DETAIL_META_HASH_HPP


#include <cstdio>

#include "transport/cryptofile.hpp"
#include "transport/detail/meta_opener.hpp"
#include "transport/detail/meta_writer.hpp"
#include "fileutils.hpp"

namespace detail {




struct HashHeader {
    unsigned version;
};

template<class Reader>
HashHeader read_hash_headers(ReaderLine<Reader> & reader)
{
    HashHeader header{1};

    char line[32];
    auto sz = reader.read_line(line, sizeof(line), ERR_TRANSPORT_READ_FAILED);
    if (sz < 0) {
        throw Error(ERR_TRANSPORT_READ_FAILED, errno);
    }

    // v2
    REDASSERT(line[0] == 'v');
    header.version = 2;

    if (reader.next_line()
     || reader.next_line()
    ) {
        throw Error(ERR_TRANSPORT_READ_FAILED, errno);
    }

    return header;
}

template<class Reader>
int read_hash_file_v2(ReaderLine<Reader> & reader, HashHeader const & /*hash_header*/, bool has_hash, MetaLine & hash_line) {
    return read_meta_file_v2_impl<false>(reader, has_hash, hash_line);
}

}

#endif

