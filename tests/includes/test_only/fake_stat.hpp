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
*   Copyright (C) Wallix 2010-2016
*   Author(s): Jonathan Poelen
*/

#pragma once

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
//fake_stat.hpp:31, gcc/clang warning, Priority: Normal
//missing initializer for member ‘stat::st_uid’

//fake_stat.hpp:31, gcc/clang warning, Priority: Normal
//missing initializer for member ‘stat::st_mode’
//fake_stat.hpp:31, gcc/clang warning, Priority: Normal
//missing initializer for member ‘stat::__pad0’
//fake_stat.hpp:31, gcc/clang warning, Priority: Normal
//missing initializer for member ‘stat::st_size’
//fake_stat.hpp:31, gcc/clang warning, Priority: Normal
//missing initializer for member ‘stat::st_ctim’
//fake_stat.hpp:31, gcc/clang warning, Priority: Normal
//missing initializer for member ‘stat::__glibc_reserved’
//fake_stat.hpp:31, gcc/clang warning, Priority: Normal
//missing initializer for member ‘stat::st_rdev’
//fake_stat.hpp:31, gcc/clang warning, Priority: Normal
//missing initializer for member ‘stat::st_dev’
//fake_stat.hpp:31, gcc/clang warning, Priority: Normal
//missing initializer for member ‘stat::st_blksize’
//fake_stat.hpp:31, gcc/clang warning, Priority: Normal
//missing initializer for member ‘stat::st_ino’
//fake_stat.hpp:31, gcc/clang warning, Priority: Normal
//missing initializer for member ‘stat::st_gid’
//fake_stat.hpp:31, gcc/clang warning, Priority: Normal
//missing initializer for member ‘stat::st_atim’
//fake_stat.hpp:31, gcc/clang warning, Priority: Normal
//missing initializer for member ‘stat::st_mtim’
//fake_stat.hpp:31, gcc/clang warning, Priority: Normal
//missing initializer for member ‘stat::st_blocks’
//fake_stat.hpp:31, gcc/clang warning, Priority: Normal
//missing initializer for member ‘stat::st_nlink’

struct FakeFstat : Fstat
{
    virtual int stat(const char * filename, struct stat & stat)
    {
        stat = {};
        return 0;
    }
};

