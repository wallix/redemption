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
 *   Copyright (C) Wallix 2010-2012
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen
 */

#if !defined(__UNLINK_WRM__)
#define __UNLINK_WRM__

#include <cstdio>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

inline void unlink_wrm(const char * path_base, pid_t pid, uint start, uint count)
{
    char filename[50];
    for (count += start; start < count; ++start)
    {
        std::sprintf(filename, "%s-%u-%u.wrm", path_base, pid, start);
        unlink(filename);
    }
}

inline void unlink_wrm(const char * path_base, pid_t pid, uint count)
{
    unlink_wrm(path_base, pid, 0, count);
}

inline void unlink_wrm(const char * path_base, uint count)
{
    unlink_wrm(path_base, getpid(), 0, count);
}

inline void unlink_breakpoint(const char * path_base, pid_t pid, uint start, uint count)
{
    char filename[50];
    for (count += start; start < count; ++start)
    {
        std::sprintf(filename, "%s-%u-%u.wrm.png", path_base, pid, start);
        unlink(filename);
    }
}

inline void unlink_breakpoint(const char * path_base, pid_t pid, uint count)
{
    unlink_breakpoint(path_base, pid, 1, count);
}

inline void unlink_breakpoint(const char * path_base, uint count)
{
    unlink_breakpoint(path_base, getpid(), 1, count);
}

inline void unlink_mwrm(const char * path_base, pid_t pid)
{
    char filename[50];
    std::sprintf(filename, "%s-%u.mwrm", path_base, pid);
    unlink(filename);
}

inline void unlink_mwrm(const char * path_base)
{
    unlink_mwrm(path_base, getpid());
}

inline void unlink_mwrm_and_wrm(const char * path_base, pid_t pid, uint start, uint count)
{
    unlink_mwrm(path_base, pid);
    unlink_wrm(path_base, pid, start, count);
    unlink_breakpoint(path_base, pid, start+1, count);
}

inline void unlink_mwrm_and_wrm(const char * path_base, pid_t pid, uint count)
{
    unlink_mwrm_and_wrm(path_base, pid, 0, count);
}

inline void unlink_mwrm_and_wrm(const char * path_base, uint count)
{
    unlink_mwrm_and_wrm(path_base, getpid(), 0, count);
}

inline void unlink_png(const char * path_base, pid_t pid, uint start, uint count)
{
    char filename[50];
    int size;
    for (count += start; start < count; ++start)
    {
        size = std::sprintf(filename, "%s-%u-%u.png", path_base, pid, start);
        unlink(filename);
        strcpy(filename + size, ".meta")[size + 5] = 0;
        unlink(filename);
    }
}

inline void unlink_png(const char * path_base, pid_t pid, uint count)
{
    unlink_png(path_base, pid, 0, count);
}

inline void unlink_png(const char * path_base, uint count)
{
    unlink_png(path_base, getpid(), 0, count);
}

#endif