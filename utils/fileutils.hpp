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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stddef.h>
#include <sys/socket.h>
#include <ctype.h>
#include "log.hpp"
#include "error.hpp"
#include "regex.hpp"

static inline int filesize(const char * path)
{
    struct stat sb;
    int status = stat(path, &sb);
    if (status >= 0){
        return sb.st_size;
    }
    return -1;
}

static inline void canonical_path( const char * fullpath, char * path, size_t path_len
                                 , char * basename, size_t basename_len, char * extension
                                 , size_t extension_len, uint32_t verbose = 255)
{
    const char * end_of_path = strrchr(fullpath, '/');
    if (end_of_path){
        if (static_cast<size_t>(end_of_path + 1 - fullpath) <= path_len) {
            memcpy(path, fullpath, end_of_path + 1 - fullpath);
            path[end_of_path + 1 - fullpath] = 0;
        }
        else if (verbose >= 255) {
            LOG(LOG_ERR, "canonical_path : Path too long for the buffer\n");
        }
        const char * start_of_extension = strrchr(end_of_path + 1, '.');
        if (start_of_extension){
            snprintf(extension, extension_len, "%s", start_of_extension);
            //strcpy(extension, start_of_extension);
            if (start_of_extension > end_of_path + 1){
                if (static_cast<size_t>(start_of_extension - end_of_path - 1) <= basename_len) {
                    memcpy(basename, end_of_path + 1, start_of_extension - end_of_path - 1);
                    basename[start_of_extension - end_of_path - 1] = 0;
                }
                else if (verbose >= 255) {
                    LOG(LOG_ERR, "canonical_path : basename too long for the buffer\n");
                }
            }
            // else no basename : leave output buffer for name untouched
        }
        else {
            if (end_of_path[1]){
                snprintf(basename, basename_len, "%s", end_of_path + 1);
                //strcpy(basename, end_of_path + 1);
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
            snprintf(extension, extension_len, "%s", start_of_extension);
            // strcpy(extension, start_of_extension);
            if (start_of_extension > fullpath){
                if (static_cast<size_t>(start_of_extension - fullpath) <= basename_len) {
                    memcpy(basename, fullpath, start_of_extension - fullpath);
                    basename[start_of_extension - fullpath] = 0;
                }
                else if (verbose >= 255) {
                    LOG(LOG_ERR, "canonical_path : basename too long for the buffer\n");
                }
            }
            // else no basename : leave output buffer for name untouched
        }
        else {
            if (fullpath[0]){
                snprintf(basename, basename_len, "%s", fullpath);
                // strcpy(basename, fullpath);
                // default extension : leave whatever is in extension output buffer
            }
            else {
                // default name : leave whatever is in name output buffer
                // default extension : leave whatever is in extension output buffer
            }
        }
    }
    if (verbose >= 255) {
        LOG(LOG_INFO, "canonical_path : %s%s%s\n", path, basename, extension);
    }
}

static inline char * pathncpy(char * dest, const char * src, const size_t n) {
    TODO("use error return value instead of raisong an exception. The returned pointer is only used in tests, not code anyway");
    size_t src_len = strnlen(src, n);
    if (src_len >= n) {
        LOG(LOG_INFO, "can't copy path, no room in dest path (available %d): %s\n", static_cast<int>(n), src);
        throw Error(ERR_PATH_TOO_LONG);
    }
    if ((src_len == 0) && (n >= 3)){
        memcpy(dest, "./", 3);
    }
    else {
        memcpy(dest, src, src_len + 1);
        if (src[src_len - 1] != '/') {
            if (src_len + 1 >= n) {
                LOG(LOG_INFO, "can't copy path, no room in dest path to add trailing slash: %s\n", src);
                throw Error(ERR_PATH_TOO_LONG);
            }
            dest[src_len] = '/';
            dest[src_len+1] = 0;
        }
    }
    return dest;
}

static inline void clear_files_flv_meta_png(const char * path, const char * prefix, uint32_t verbose = 255)
{
    DIR * d = opendir(path);
    if (d){
//        char static_buffer[8192];
        char buffer[8192];
        size_t path_len = strlen(path);
        size_t prefix_len = strlen(prefix);
        size_t file_len = 1024;
//        size_t file_len = pathconf(path, _PC_NAME_MAX) + 1;
//        char * buffer = static_buffer;

/*
        if (file_len < 4000){
            if (verbose >= 255) {
                LOG(LOG_WARNING, "File name length is in normal range (%u), using static buffer", static_cast<unsigned>(file_len));
            }
            file_len = 4000;
        }
        else {
            if (verbose >= 255) {
                LOG(LOG_WARNING, "Max file name too large (%u), using dynamic buffer", static_cast<unsigned>(file_len));
            }

            char * buffer = (char*)malloc(file_len + path_len + 1);
            if (!buffer){
                if (verbose >= 255) {
                    LOG(LOG_WARNING, "Memory allocation failed for file name buffer, using static buffer");
                }
                buffer = static_buffer;
                file_len = 4000;
            }
        }
*/
        if (file_len + path_len + 1 > sizeof(buffer)) {
            LOG(LOG_WARNING, "Path len %u > %u", file_len + path_len + 1, sizeof(buffer));
            return;
        }
        strncpy(buffer, path, file_len + path_len + 1);
        if (buffer[path_len] != '/'){
            buffer[path_len] = '/'; path_len++; buffer[path_len] = 0;
        }

        size_t len = offsetof(struct dirent, d_name) + file_len;
        struct dirent * entryp = static_cast<struct dirent *>(malloc(len));
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
                if (verbose >= 255) {
                    LOG(LOG_WARNING, "Failed to read file %s [%u: %s]\n", buffer, errno, strerror(errno));
                }
                continue;
            }
            if (unlink(buffer) < 0){
                LOG(LOG_WARNING, "Failed to remove file %s", buffer, errno, strerror(errno));
            }
        }
        closedir(d);
        free(entryp);
/*
        if (buffer != static_buffer){
            free(buffer);
        }
*/
    }
    else {
        LOG(LOG_WARNING, "Failed to open directory %s [%u: %s]", path, errno, strerror(errno));
    }
}

static inline int _internal_make_directory(const char *directory, mode_t mode, const int groupid, uint32_t verbose) {
    struct stat st;
    int status = 0;

    if ((directory[0] != 0) && strcmp(directory, ".") && strcmp(directory, "..")) {
        if (stat(directory, &st) != 0) {
            /* Directory does not exist. */
            if ((mkdir(directory, mode) != 0) && (errno != EEXIST)) {
                status = -1;
                if (verbose >= 255) {
                    LOG(LOG_ERR, "failed to create directory %s : %s [%u]", directory, strerror(errno), errno);
                }
            }
            if (groupid){
                if (chown(directory, static_cast<uid_t>(-1), groupid) < 0){
                    if (verbose >= 255) {
                        LOG(LOG_ERR, "can't set directory %s group to %u : %s [%u]", directory, groupid, strerror(errno), errno);
                    }
                }
            }

        }
        else if (!S_ISDIR(st.st_mode)) {
            errno = ENOTDIR;
            if (verbose >= 255) {
                LOG(LOG_ERR, "expecting directory name, got filename, for %s");
            }
            status = -1;
        }
    }
    return status;
}



TODO("Add unit tests for recursive_create_directory");
static inline int recursive_create_directory(const char *directory, mode_t mode, const int groupid, uint32_t verbose = 255) {
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
        status = _internal_make_directory(copy_directory, mode, groupid, verbose);
        *pSearch = '/';
    }

    if (status == 0) {
        status = _internal_make_directory(directory, mode, groupid, verbose);
    }

    free(copy_directory);

    return status;
}


struct LineBuffer
{
    char buffer[20480];
    int end_buffer;

    int fd;
    int begin_line;
    int eol;

    LineBuffer(int fd)
    : end_buffer(0)
    , fd(fd)
    , begin_line(0)
    , eol(0)
    {
    }

    int readline()
    {
        for (int i = this->begin_line; i < this->end_buffer; i++){
            if (this->buffer[i] == '\n'){
                this->eol = i+1;
                return 1;
            }
        }
        size_t trailing_room = sizeof(this->buffer) - this->end_buffer;
        // reframe buffer if no trailing room left
        if (trailing_room == 0){
            size_t used_len = this->end_buffer - this->begin_line;
            memmove(this->buffer, &(this->buffer[this->begin_line]), used_len);
            this->end_buffer = used_len;
            this->begin_line = 0;
        }

        ssize_t res = read(this->fd, &(this->buffer[this->end_buffer]), sizeof(this->buffer) - this->end_buffer);
        if (res < 0){
            return res;
        }
        this->end_buffer += res;
        if (this->begin_line == this->end_buffer) {
            return 0;
        }
        for (int i = this->begin_line; i < this->end_buffer; i++){
            if (this->buffer[i] == '\n'){
                this->eol = i+1;
                return 1;
            }
        }
        this->eol = this->end_buffer;
        return 1;
    }
};

// return 0 if found, -1 not found or error
static inline int parse_ip_conntrack(int fd, const char * source, const char * dest, int sport, int dport, char * transparent_dest, size_t sz_transparent_dest, uint32_t verbose = 0)
{
    LineBuffer line(fd);
    //"tcp      6 299 ESTABLISHED src=10.10.43.13 dst=10.10.47.93 sport=36699 dport=22 packets=5256 bytes=437137 src=10.10.47.93 dst=10.10.43.13 sport=22 dport=36699 packets=3523 bytes=572101 [ASSURED] mark=0 secmark=0 use=2\n"

    char strre[512];
#define RE_IP_DEF "\\d\\d?\\d?\\.\\d\\d?\\d?\\.\\d\\d?\\d?\\.\\d\\d?\\d?"
    sprintf(strre,
            "^ *6 +\\d+ +ESTABLISHED +"
            "src="RE_IP_DEF" +"
            "dst=("RE_IP_DEF") +"
            "sport=\\d+ +dport=\\d+ +packets=\\d+ bytes=\\d+ +"
            "src=%s +"
            "dst=%s +"
            "sport=%d +dport=%d +packets=\\d+ bytes=\\d+ +"
            "\\[ASSURED] +mark=\\d+ +secmark=\\d+ use=\\d+$",
            source, dest, sport, dport
    );
#undef RE_IP_DEF
    re::Regex regex(strre);

    int status = line.readline();
    for (; status == 1 ; (line.begin_line = line.eol), (status = line.readline())) {
        if (verbose) {
            fprintf(stderr, "Line: %.*s", line.eol - line.begin_line, &line.buffer[line.begin_line]);
        }

        if (line.eol - line.begin_line < 100) {
            continue;
        }

        const char * s = line.buffer + line.begin_line;
        if (s[0] != 't' || s[1] != 'c' || s[2] != 'p' || s[3] != ' ') {
            continue ;
        }
        s += 4;

        const bool contains_endl = line.buffer[line.eol-1] == '\n';
        if (contains_endl) {
            line.buffer[line.eol-1] = 0;
        }

        re::Regex::range_matches matches = regex.exact_match(s);
        if ( ! matches.empty() ) {
            const size_t match_size = matches[0].second - matches[0].first;
            if (match_size >= sz_transparent_dest){
                LOG(LOG_WARNING, "No enough space to store transparent ip target address");
                return -1;
            }

            memcpy(transparent_dest, matches[0].first, match_size);
            transparent_dest[match_size] = 0;

            if (verbose) {
                LOG(LOG_INFO, "Match found: %s", transparent_dest);
            }

            return 0;
        }

        if (contains_endl) {
            line.buffer[line.eol-1] = '\n';
        }
    }
    // transparent ip route not found in ip_conntrack
    return -1;
}

#endif
