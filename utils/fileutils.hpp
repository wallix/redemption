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

static inline int filesize(const char * path)
{
    struct stat sb;
    int status = stat(path, &sb);
    if (status >= 0){
        return sb.st_size;
    }
    return -1;
}

static inline bool canonical_path( const char * fullpath, char * path, size_t path_len
                                 , char * basename, size_t basename_len, char * extension
                                 , size_t extension_len, uint32_t verbose = 255)
{
    const char * end_of_path = strrchr(fullpath, '/');
    if (end_of_path){
        if (static_cast<size_t>(end_of_path + 1 - fullpath) <= path_len) {
            memcpy(path, fullpath, end_of_path + 1 - fullpath);
            path[end_of_path + 1 - fullpath] = 0;
        }
        else {
            if (verbose >= 255) {
                LOG(LOG_ERR, "canonical_path : Path too long for the buffer\n");
            }
            return false;
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
                else {
                    if (verbose >= 255) {
                        LOG(LOG_ERR, "canonical_path : basename too long for the buffer\n");
                    }
                    return false;
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
                else {
                    if (verbose >= 255) {
                        LOG(LOG_ERR, "canonical_path : basename too long for the buffer\n");
                    }
                    return false;
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
    return true;
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
    int eollen;

    int begin_word;
    int eow;

    LineBuffer(int fd)
        : end_buffer(0)
        , fd(fd)
        , begin_line(0)
        , eol(0)
        , eollen(1)
        , begin_word(0)
        , eow(0)
    {
    }

    int readline()
    {
        for (int i = this->begin_line; i < this->end_buffer; i++){
            if (this->buffer[i] == '\n'){
                this->eol = i+1;
                this->eollen = 1;
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
                this->eollen = 1;
                return 1;
            }
        }
        this->eol = this->end_buffer;
        this->eollen = 0;
        return 1;
    }

    int get_protocol()
    {
        int res = -1;
        int i = this->begin_word;
        for ( ; i < this->eol ; i++){
            char c = this->buffer[i];
            if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || (c == '-'))){
                if (isspace(c)){
                    res = 0;
                }
                break;
            }
        }
        this->eow = i;
        return res;
    }

    int get_status1()
    {
        int res = -1;
        int i = this->begin_word;
        for ( ; i < this->eol ; i++){
            char c = this->buffer[i];
            if (!(c >= 'A' && c <= 'Z')){
                if (isspace(c)){
                    res = 0;
                }
                break;
            }
        }
        this->eow = i;
        return res;
    }

    int get_space()
    {
        int i = this->begin_word;
        int res = -1;
        char c = this->buffer[i];
        if (isspace(c)){
            res = 0;
            i++;
            for ( ; i < this->eol ; i++){
                char c = this->buffer[i];
                if (!isspace(c)){
                    break;
                }
            }
        }
        this->eow = i;
        return res;

    }

    int get_num(int start)
    {
        int res = -1;
        int i = start;
        for ( ; i < this->eol ; i++){
            char c = this->buffer[i];
            if (!(c >= '0' && c <= '9')){
                break;
            }
            res = 0;
        }
        this->eow = i;
        return res;
    }

    int get_protocol_number()
    {
        int res = this->get_num(this->begin_word);
        if (this->eow == this->eol){
            return res;
        }
        if (isspace(this->buffer[this->eow])){
            return res;
        }
        return -1;
    }

    int get_ttl_sec()
    {
        int res = this->get_num(this->begin_word);
        if (this->eow == this->eol){
            return res;
        }
        if (isspace(this->buffer[this->eow])){
            return res;
        }
        return -1;
    }

    int get_identifier()
    {
        int res = -1;
        int i = this->begin_word;
        for ( ; i < this->eol ; i++){
            char c = this->buffer[i];
            if (!(c >= 'a' && c <= 'z')){
                break;
            }
            res = 0;
        }
        this->eow = i;
        return res;
    }

    int get_var()
    {
        int res = this->get_identifier();
        if (res < 0){
            return res;
        }
        if (this->eow + 1 >= this->eol){
            return -1;
        }
        if (this->buffer[this->eow] != '='){
            return -1;
        }
        this->eow++;
        return 0;
    }

    int get_ip()
    {
        int res = this->get_num(this->begin_word);
        if (res < 0){
            return -1;
        }
        if (this->eow + 1 >= this->eol){
            return -1;
        }
        if (this->buffer[this->eow] != '.'){
            return -1;
        }
        this->eow++;
        res = this->get_num(this->eow);
        if (res < 0){
            return -1;
        }
        if (this->eow + 1 >= this->eol){
            return -1;
        }
        if (this->buffer[this->eow] != '.'){
            return -1;
        }
        this->eow++;
        res = this->get_num(this->eow);
        if (res < 0){
            return -1;
        }
        if (this->eow + 1 >= this->eol){
            return -1;
        }
        if (this->buffer[this->eow] != '.'){
            return -1;
        }
        this->eow++;
        return this->get_num(this->eow);
    }


    int get_var_ip()
    {
        int res = this->get_var();
        if (res < 0){
            return res;
        }
        res = this->get_num(this->eow);
        if (res < 0){
            return res;
        }
        if (this->eow + 1 >= this->eol){
            return -1;
        }
        if (this->buffer[this->eow] != '.'){
            return -1;
        }
        this->eow++;
        res = this->get_num(this->eow);
        if (res < 0){
            return res;
        }
        if (this->eow + 1 >= this->eol){
            return -1;
        }
        if (this->buffer[this->eow] != '.'){
            return -1;
        }
        this->eow++;
        res = this->get_num(this->eow);
        if (res < 0){
            return res;
        }
        if (this->eow + 1 >= this->eol){
            return -1;
        }
        if (this->buffer[this->eow] != '.'){
            return -1;
        }
        this->eow++;
        res = this->get_num(this->eow);
        if ((this->eow <= this->eol) && (!isspace(this->buffer[this->eow]))){
            return -1;
        }
        return res;
    }

    int get_src_ip()
    {
        int res = this->get_var_ip();
        if (res < 0){
            return -1;
        }
        if (this->eow - this->begin_word < 4){
            return -1;
        }
        if (0 != memcmp(&this->buffer[this->begin_word], "src=", 4)){
            return -1;
        }
        return 0;
    }

    int get_dst_ip()
    {
        int res = this->get_var_ip();
        if (res < 0){
            return -1;
        }
        if (this->eow - this->begin_word < 4){
            return -1;
        }
        if (0 != memcmp(&this->buffer[this->begin_word], "dst=", 4)){
            return -1;
        }
        return 0;
    }

    int get_var_num()
    {
        int res = this->get_var();
        if (res < 0){
            return res;
        }
        res = this->get_num(this->eow);
        if (res < 0){
            return res;
        }
        if ((this->eow >= this->eol) || (!isspace(this->buffer[this->eow]))){
            return -1;
        }
        return res;
    }

    int get_sport()
    {
        int res = this->get_var_num();
        if (res < 0){
            return -1;
        }
        if (this->eow - this->begin_word < 6){
            return -1;
        }
        if (0 != memcmp(&this->buffer[this->begin_word], "sport=", 6)){
            return -1;
        }
        return 0;
    }

    int get_dport()
    {
        int res = this->get_var_num();
        if (res < 0){
            return -1;
        }
        if (this->eow - this->begin_word < 6){
            return -1;
        }
        if (0 != memcmp(&this->buffer[this->begin_word], "dport=", 6)){
            return -1;
        }
        return 0;
    }

    int get_packets()
    {
        int res = this->get_var_num();
        if (res < 0){
            return -1;
        }
        if (this->eow - this->begin_word < 8){
            return -1;
        }
        if (0 != memcmp(&this->buffer[this->begin_word], "packets=", 8)){
            return -1;
        }
        return 0;
    }

    int get_bytes()
    {
        int res = this->get_var_num();
        if (res < 0){
            return -1;
        }
        if (this->eow - this->begin_word < 6){
            return -1;
        }
        if (0 != memcmp(&this->buffer[this->begin_word], "bytes=", 6)){
            return -1;
        }
        return 0;
    }

    int get_status()
    {
        int res = -1;
        int i = this->begin_word;
        if (i >= this->eol){
            return -1;
        }
        char c = this->buffer[i];
        if (c != '['){
            return -1;
        }
        i++;

        for ( ; i < this->eol ; i++){
            char c = this->buffer[i];
            if (!(c >= 'A' && c <= 'Z')){
                break;
            }
            res = 0;
        }
        if (res < 0){
            return -1;
        }
        if (i >= this->eol){
            return -1;
        }
        c = this->buffer[i];
        if (c != ']'){
            return -1;
        }
        i++;
        this->eow = i;
        return res;
    }

    int get_mark()
    {
        int res = this->get_var_num();
        if (res < 0){
            return -1;
        }
        if (this->eow - this->begin_word < 5){
            return -1;
        }
        if (0 != memcmp(&this->buffer[this->begin_word], "mark=", 5)){
            return -1;
        }
        return 0;
    }

    int get_secmark()
    {
        int res = this->get_var_num();
        if (res < 0){
            return -1;
        }
        if (this->eow - this->begin_word < 8){
            return -1;
        }
        if (0 != memcmp(&this->buffer[this->begin_word], "secmark=", 8)){
            return -1;
        }
        return 0;
    }

    int get_use()
    {
        int res = this->get_var_num();
        if (res < 0){
            return -1;
        }
        if (this->eow - this->begin_word < 4){
            return -1;
        }
        if (0 != memcmp(&this->buffer[this->begin_word], "use=", 4)){
            return -1;
        }
        return 0;
    }
};

// return 0 if found, -1 not found or error
static inline int parse_ip_conntrack(int fd, const char * source, const char * dest, int sport, int dport, char * transparent_dest, int sz_transparent_dest, uint32_t verbose = 0)
{
    LineBuffer line(fd);
    char src_port[6];
    char dst_port[6];
    int len_src_port = sprintf(src_port, "%u", sport);
    int len_dst_port = sprintf(dst_port, "%u", dport);
    int len_source = strlen(source);
    int len_dest = strlen(dest);

    int status = line.readline();

    //"tcp      6 299 ESTABLISHED src=10.10.43.13 dst=10.10.47.93 sport=36699 dport=22 packets=5256 bytes=437137 src=10.10.47.93 dst=10.10.43.13 sport=22 dport=36699 packets=3523 bytes=572101 [ASSURED] mark=0 secmark=0 use=2\n"

    char tmp_transparent_dest[64] = {};
    int len_tmp_transparent_dest = 0;

    for ( ; status == 1 ; (line.begin_line = line.eol), (status = line.readline())) {
        if (verbose) {
            LOG(LOG_INFO, "Line: %.*s", line.eol - line.begin_line, &line.buffer[line.begin_line]);
        }

        line.begin_word = line.begin_line;

        // tcp
        if (line.get_protocol() < 0) { continue; }
//        printf("Word: %.*s\n", line.eow - line.begin_word, &line.buffer[line.begin_word]);
        if ((line.eow - line.begin_word != 3)
        || (0 != memcmp(&line.buffer[line.begin_word], "tcp", 3))){ continue; }
        line.begin_word = line.eow;
        if (line.get_space() < 0) { continue; }
        line.begin_word = line.eow;

        // 6
        if (line.get_protocol_number() < 0) { continue; }
//        printf("Word: %.*s\n", line.eow - line.begin_word, &line.buffer[line.begin_word]);
        if ((line.eow - line.begin_word != 1)
        || (0 != memcmp(&line.buffer[line.begin_word], "6", 1))){ continue; }
        line.begin_word = line.eow;
        if (line.get_space() < 0) { continue; }
        line.begin_word = line.eow;

        // 0
        if (line.get_ttl_sec() < 0) { continue; }
//        printf("Word TTL: %.*s\n", line.eow - line.begin_word, &line.buffer[line.begin_word]);
        line.begin_word = line.eow;
        if (line.get_space() < 0) { continue; }
        line.begin_word = line.eow;

        // ESTABLISHED
        if (line.get_status1() < 0) { continue; }
//        printf("Word: %.*s\n", line.eow - line.begin_word, &line.buffer[line.begin_word]);
        if ((line.eow - line.begin_word != 11)
        || (0 != memcmp(&line.buffer[line.begin_word], "ESTABLISHED", 11))){ continue; }
        line.begin_word = line.eow;
        if (line.get_space() < 0) { continue; }
        line.begin_word = line.eow;

        // src=10.10.43.31
        if (line.get_src_ip() < 0) { continue; }
        line.begin_word = line.eow;
        if (line.get_space() < 0) { continue; }
        line.begin_word = line.eow;

        // dst=10.10.47.255
        if (line.get_dst_ip() < 0) { continue; }
        if (line.eow - line.begin_word > static_cast<int>(sizeof(tmp_transparent_dest) + 4)){
            return -1;
        }
        len_tmp_transparent_dest = line.eow - line.begin_word - 4;
        memcpy(tmp_transparent_dest, &line.buffer[line.begin_word + 4], len_tmp_transparent_dest);
        line.begin_word = line.eow;
        if (line.get_space() < 0) { continue; }
        line.begin_word = line.eow;

        // sport=57621
        if (line.get_sport() < 0) { continue; }
        line.begin_word = line.eow;
        if (line.get_space() < 0) { continue; }
        line.begin_word = line.eow;

        // dport=57621
        if (line.get_dport() < 0) { continue; }
        line.begin_word = line.eow;
        if (line.get_space() < 0) { continue; }
        line.begin_word = line.eow;

        // packets=1139
        if (line.get_packets() == 0) {
            line.begin_word = line.eow;
            if (line.get_space() < 0) { continue; }
            line.begin_word = line.eow;
         }

        // bytes=82008
        if (line.get_bytes() == 0) {
            line.begin_word = line.eow;
            if (line.get_space() < 0) { continue; }
            line.begin_word = line.eow;
        }

        // src=10.10.47.255
        if (line.get_src_ip() < 0) { continue; }
        if ((line.eow - line.begin_word != len_source + 4)
        || (0 != memcmp(&line.buffer[line.begin_word + 4], source, len_source))){ continue; }
        line.begin_word = line.eow;
        if (line.get_space() < 0) { continue; }
        line.begin_word = line.eow;


        // dst=10.10.43.31
        if (line.get_dst_ip() < 0) { continue; }
        if ((line.eow - line.begin_word != len_dest + 4)
        || (0 != memcmp(&line.buffer[line.begin_word + 4], dest, len_dest))){ continue; }
        line.begin_word = line.eow;
        if (line.get_space() < 0) { continue; }
        line.begin_word = line.eow;

        // sport=57621
        if (line.get_sport() < 0) { continue; }
        if ((line.eow - line.begin_word != len_src_port + 6)
        || (0 != memcmp(&line.buffer[line.begin_word + 6], src_port, len_src_port))){ continue; }
        line.begin_word = line.eow;
        if (line.get_space() < 0) { continue; }
        line.begin_word = line.eow;

        // dport=57621
        if (line.get_dport() < 0) { continue; }
        if ((line.eow - line.begin_word != len_dst_port + 6)
        || (0 != memcmp(&line.buffer[line.begin_word + 6], dst_port, len_dst_port))){ continue; }
        line.begin_word = line.eow;
        if (line.get_space() < 0) { continue; }
        line.begin_word = line.eow;

        // packets=0
        if (line.get_packets() == 0) {
            line.begin_word = line.eow;
            if (line.get_space() < 0) { continue; }
            line.begin_word = line.eow;
        }

        // bytes=0
        if (line.get_bytes() == 0) {
            line.begin_word = line.eow;
            if (line.get_space() < 0) { continue; }
            line.begin_word = line.eow;
        }

        // [ASSURED]
        if (line.get_status() < 0) { continue; }
        line.begin_word = line.eow;
        if (line.get_space() < 0) { continue; }
        line.begin_word = line.eow;

        // mark=0
        if (line.get_mark() < 0) { continue; }
        line.begin_word = line.eow;
        if (line.get_space() < 0) { continue; }
        line.begin_word = line.eow;

        // secmark=0
        if (line.get_secmark() == 0) {
            line.begin_word = line.eow;
            if (line.get_space() < 0) { continue; }
            line.begin_word = line.eow;
        }

        // use=2\n";
        if (line.get_use() < 0) { continue; }
        line.begin_word = line.eow;
        if (line.get_space() < 0) { continue; }

        if (len_tmp_transparent_dest >= sz_transparent_dest){
            LOG(LOG_WARNING, "No enough space to store transparent ip target address");
            return -1;
        }
        memcpy(transparent_dest, tmp_transparent_dest, len_tmp_transparent_dest);
        transparent_dest[len_tmp_transparent_dest] = 0;

        if (verbose) {
            LOG(LOG_INFO, "Match found: %s", transparent_dest);
        }

        return 0;
    }
    // transparent ip route not found in ip_conntrack
    return -1;
}

#endif
