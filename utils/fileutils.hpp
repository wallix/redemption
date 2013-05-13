/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Product name: redemption, a FLOSS RDP proxy
   Copyright (C) Wallix 2013
   Author(s): Christophe Grosjean, Raphael Zhou

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

static inline void canonical_path(const char * fullpath, char * path, size_t path_len, char * basename, size_t basename_len, char * extension, size_t extension_len)
{
    TODO("add overflow checking of path and basename len")
    const char * end_of_path = strrchr(fullpath, '/');
    if (end_of_path){
        memcpy(path, fullpath, end_of_path + 1 - fullpath);
        path[end_of_path + 1 - fullpath] = 0;
        const char * start_of_extension = strrchr(end_of_path + 1, '.');
        if (start_of_extension){
            strcpy(extension, start_of_extension); 
            if (start_of_extension > end_of_path + 1){
                memcpy(basename, end_of_path + 1, start_of_extension - end_of_path - 1);
                basename[start_of_extension - end_of_path - 1] = 0;
            }
            // else no basename : leave output buffer for name untouched
        }
        else {
            if (end_of_path[1]){
                strcpy(basename, end_of_path + 1);
                // default extension : leave whatever is in extension output buffer
            }
            else {
                // default name : leave whatever is in name output buffer
                // default extension : leave whatever is in extension output buffer
            }
        }
    }
    else {
        // default path : leave whatever is in path output buffer
        const char * start_of_extension = strrchr(fullpath, '.');
        if (start_of_extension){
            strcpy(extension, start_of_extension); 
            if (start_of_extension > fullpath){
                memcpy(basename, fullpath, start_of_extension - fullpath);
                basename[start_of_extension - fullpath] = 0;
            }
            // else no basename : leave output buffer for name untouched
        }
        else {
            if (fullpath[0]){
                strcpy(basename, fullpath);
                // default extension : leave whatever is in extension output buffer
            }
            else {
                // default name : leave whatever is in name output buffer
                // default extension : leave whatever is in extension output buffer
            }
        }
    }
    LOG(LOG_INFO, "canonical_path : %s%s%s\n", path, basename, extension);
}

void clear_files_flv_meta_png(const char * path, const char * prefix)
{
    DIR * d = opendir(path);
    if (d){
        char static_buffer[8192];
        size_t path_len = strlen(path);
        size_t prefix_len = strlen(prefix);
        size_t file_len = pathconf(path, _PC_NAME_MAX) + 1;
        char * buffer = static_buffer;
/*        
        if ((file_len > 4000) || (file_len < 10)){
            LOG(LOG_WARNING, "Max file name too large or undefined (%u) using static buffer", (unsigned)file_len);
            file_len = 4000;
        }
*/        
        if (file_len < 4000){
            LOG(LOG_WARNING, "File name length is in normal range (%u), using static buffer", (unsigned)file_len);
            file_len = 4000;
        }
        else {
            LOG(LOG_WARNING, "Max file name too large (%u), using dynamic buffer", (unsigned)file_len);

            char * buffer = (char*)malloc(file_len + path_len + 1);
            if (!buffer){
                LOG(LOG_WARNING, "Memory allocation failed for file name buffer, using static buffer");
                buffer = static_buffer;
                file_len = 4000;
            }
        }
        strncpy(buffer, path, file_len + path_len + 1);
        if (buffer[path_len] != '/'){
            buffer[path_len] = '/'; path_len++; buffer[path_len] = 0;
        }

        size_t len = offsetof(struct dirent, d_name) + file_len;
        struct dirent * entryp = (struct dirent *)malloc(len);
        if (!entryp){
            LOG(LOG_WARNING, "Memory allocation failed for entryp, exiting file cleanup code");
            return;
        }
        struct dirent * result;
        for (readdir_r(d, entryp, &result) ; result ; readdir_r(d, entryp, &result)) {
            if ((0 == strcmp(entryp->d_name, ".")) || (0 == strcmp(entryp->d_name, ".."))){
                continue;
            }

            if (strncmp(entryp->d_name, prefix, prefix_len)){
                continue;
            }

            strncpy(buffer + path_len, entryp->d_name, file_len);
            const char * eob = buffer + path_len + strlen(entryp->d_name);
            const bool extension = ((eob[-4] == '.') && (eob[-3] == 'f') && (eob[-2] == 'l') && (eob[-1] == 'v'))
                          || ((eob[-4] == '.') && (eob[-3] == 'p') && (eob[-2] == 'n') && (eob[-1] == 'g'))
                          || ((eob[-5] == '.') && (eob[-4] == 'm') && (eob[-3] == 'e') && (eob[-2] == 't') && (eob[-1] == 'a'))
                          ;

            if (!extension){
                continue;
            }

            struct stat st;
            if (stat(buffer, &st) < 0){
                LOG(LOG_WARNING, "Failed to read file %s [%u: %s]\n", buffer, errno, strerror(errno));
                continue;
            }
            if (unlink(buffer) < 0){
                LOG(LOG_WARNING, "Failed to remove file %s", buffer, errno, strerror(errno));
            }
        }
        closedir(d);
        free(entryp);
        if (buffer != static_buffer){
            free(buffer);
        }
    }
    else {
        LOG(LOG_WARNING, "Failed to open directory %s [%u: %s]", path, errno, strerror(errno));
    }
}

static inline int _internal_make_directory(const char *directory, mode_t mode, const int groupid) {
    struct stat st;
    int status = 0;

    if ((directory[0] != 0) && strcmp(directory, ".") && strcmp(directory, "..")) {
        if (stat(directory, &st) != 0) {
            /* Directory does not exist. */
            if ((mkdir(directory, mode) != 0) && (errno != EEXIST)) {
                status = -1;
                LOG(LOG_ERR, "failed to create directory %s : %s [%u]", directory, strerror(errno), errno);
            }
            if (groupid){
                if (chown(directory, (uid_t)-1, groupid) < 0){
                    LOG(LOG_ERR, "can't set directory %s group to %u : %s [%u]", directory, groupid, strerror(errno), errno);
                }
            }

        }
        else if (!S_ISDIR(st.st_mode)) {
            errno = ENOTDIR;
            LOG(LOG_ERR, "expecting directory name, got filename, for %s");
            status = -1;
        }
    }
    return status;
}

TODO("Add unit tests for recursive_create_directory")
static inline int recursive_create_directory(const char *directory, mode_t mode, const int groupid) {
    int    status;
    char * copy_directory;
    char * pTemp;
    char * pSearch;

    status         = 0;
    copy_directory = strdup(directory);

    if (copy_directory == NULL) {
        return -1;
    }

    for ( pTemp = copy_directory
        ; (status == 0) && ((pSearch = strchr(pTemp, '/')) != 0)
        ; pTemp = pSearch + 1) {
        if (pSearch == pTemp) {
            continue;
        }

        pSearch[0] = 0;
        status = _internal_make_directory(copy_directory, mode, groupid);
        *pSearch = '/';
    }

    if (status == 0) {
        status = _internal_make_directory(directory, mode, groupid);
    }

    free(copy_directory);

    return status;
}

#endif
