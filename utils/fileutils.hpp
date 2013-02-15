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

   File related utility functions

*/

#ifndef _REDEMPTION_UTILS_FILEUTILS_HPP_
#define _REDEMPTION_UTILS_FILEUTILS_HPP_


#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stddef.h>
#include "log.hpp"

static inline int filesize(const char * path)
{
    struct stat sb;
    int status = stat(path, &sb);
    if (status >= 0){
        return sb.st_size;
    }
    return -1;
}

static inline void canonical_path(const char * fullpath, char * path, size_t path_len, char * basename, size_t basename_len)
{
    TODO("add parameters values for default path and basename. From inifile ?")
    TODO("add extraction of extension")
    TODO("add overflow checking of path and basename len")
    const char * end_of_path = strrchr(fullpath, '/');
    if (end_of_path){
        memcpy(path, fullpath, end_of_path + 1 - fullpath);
        path[end_of_path + 1 - fullpath] = 0;
        const char * start_of_extension = strrchr(end_of_path + 1, '.');
        if (start_of_extension){
            memcpy(basename, end_of_path + 1, start_of_extension - end_of_path - 1);
            basename[start_of_extension - end_of_path - 1] = 0;
        }
        else {
            if (end_of_path[0]){
                strcpy(basename, end_of_path + 1);
            }
	    else {
	      strcpy(basename, "no_name");
	    }
        }
    }
    else {
      strcpy(path, "./");
        const char * start_of_extension = strrchr(fullpath, '.');
        if (start_of_extension){
            memcpy(basename, fullpath, start_of_extension - fullpath);
            basename[start_of_extension - fullpath] = 0;
        }
        else {
            if (fullpath[0]){
                strcpy(basename, fullpath);
            }
        else {
            strcpy(basename, "no_name");
        }
        }
    }
    LOG(LOG_INFO, "canonical_path : %s%s\n", path, basename);
}

#endif
