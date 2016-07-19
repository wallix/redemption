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
 *   Copyright (C) Wallix 2010-2013
 *   Author(s): Christophe Grosjean, Raphael Zhou, Jonathan Poelen, Meng Tan
 */


#pragma once

#include <cerrno>
#include <fcntl.h>
#include <snappy-c.h>
#include <stdint.h>
#include <unistd.h>
#include <algorithm>
#include <cerrno>
#include <cstdio>

#include <memory>


#include "utils/log.hpp"
#include "core/error.hpp"
#include "no_param.hpp"
#include "utils/fileutils.hpp"
#include "openssl_crypto.hpp"
#include "transport/cryptofile.hpp"

namespace detail
{
    inline char chex_to_int(char c, int & err) {
        return
            '0' <= c && c <= '9' ? c-'0'
          : 'a' <= c && c <= 'f' ? c-'a' + 10
          : 'A' <= c && c <= 'F' ? c-'A' + 10
          : ((err |= 1), '\0');
    }

    struct MetaHeader {
        unsigned version;
        //unsigned witdh;
        //unsigned height;
        bool has_checksum;
    };

    struct MetaLine
    {
        char    filename[PATH_MAX + 1];
        off_t   size;
        mode_t  mode;
        uid_t   uid;
        gid_t   gid;
        dev_t   dev;
        ino_t   ino;
        time_t  mtime;
        time_t  ctime;
        time_t  start_time;
        time_t  stop_time;
        unsigned char hash1[MD_HASH_LENGTH];
        unsigned char hash2[MD_HASH_LENGTH];
    };

    struct temporary_concat
    {
        char str[1024];

        temporary_concat(const char * a, const char * b)
        {
            if (std::snprintf(this->str, sizeof(this->str), "%s%s", a, b) >= int(sizeof(this->str))) {
                throw Error(ERR_TRANSPORT);
            }
        }

        const char * c_str() const noexcept
        { return this->str; }
    };
}

